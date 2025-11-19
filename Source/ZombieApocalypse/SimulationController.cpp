// Copyright University of Inland Norway

#include "SimulationController.h"
#include "Engine/DataTable.h"
#include "Person.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

ASimulationController::ASimulationController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASimulationController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!PopulationDensityEffectTable)
	{
		UE_LOG(LogTemp, Error, TEXT("PopulationDensityEffectTable is not assigned!"));
	}
	else
	{
		ReadDataFromTableToVectors();
	}
	
	Conveyor.clear();
	Bitten = 0.f;
	
	SpawnGrid();
}

void ASimulationController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AccumulatedTime += DeltaTime;
	
	if (AccumulatedTime >= SimulationStepTime)
	{
		AccumulatedTime -= SimulationStepTime;
		StepSimulation();
		TimeStepsFinished++;
	}
}

void ASimulationController::SpawnGrid()
{
	UWorld* World = GetWorld();
	if (!World || !PersonClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SimulationController: World or PersonClass missing, cannot spawn grid."));
		return;
	}

	HealthyPeople.Empty();
	BittenPeople.Empty();
	ZombiePeople.Empty();

	// Spawn GridSizeX * GridSizeY people in a grid
	for (int32 y = 0; y < GridSizeY; ++y)
	{
		for (int32 x = 0; x < GridSizeX; ++x)
		{
			const FVector Location = GridOrigin + FVector(x * CellSpacing, y * CellSpacing, 0.f);
			const FRotator Rotation = FRotator::ZeroRotator;

			if (APerson* Person = World->SpawnActor<APerson>(PersonClass, Location, Rotation))
			{
				Person->SetState(EPersonState::Healthy);
				HealthyPeople.Add(Person);

				// Listen for when a person is destroyed in blueprint event graph
				Person->OnDestroyed.AddDynamic(this, &ASimulationController::OnPersonDestroyed);
			}
		}
	}

	// Spawn patient zero
	if (APerson* PatientZero = World->SpawnActor<APerson>(PersonClass, FVector::ZeroVector, FRotator::ZeroRotator))
	{
		PatientZero->SetState(EPersonState::Zombie);
		ZombiePeople.Add(PatientZero);
		
		// Listen for when a person is destroyed in blueprint event graph
		PatientZero->OnDestroyed.AddDynamic(this, &ASimulationController::OnPersonDestroyed);
	}

	// Sync stocks with visual counts
	Susceptible = HealthyPeople.Num();
	Zombies = ZombiePeople.Num();
	Bitten = BittenPeople.Num();

	UE_LOG(LogTemp, Log, TEXT("Spawned %d healthy, %d zombies visually."),
		   HealthyPeople.Num(), ZombiePeople.Num());
}

void ASimulationController::OnPersonDestroyed(AActor* DestroyedActor)
{
	APerson* Person = Cast<APerson>(DestroyedActor);
	if (!Person)
	{
		return;
	}

	/*if (Person->OnDestroyed == HealthyPeople)
	{
		InnocentKills += 1;
	}

	if (Bitten || Zombies)
	{
		ZombieKills += 1;
	}*/

	// If you want the simplest check that matches your current arrays:
	if (HealthyPeople.Contains(Person))
	{
		InnocentKills += 1;                // increment the HUD counter for innocents
		HealthyPeople.RemoveSingle(Person);
	}
	else if (BittenPeople.Contains(Person))
	{
		InfectedKills += 1;
		BittenPeople.RemoveSingle(Person);
	}
	else if (ZombiePeople.Contains(Person))
	{
		ZombieKills += 1;
		ZombiePeople.RemoveSingle(Person);
	}


	// Remove from all state arrays (only one will actually contain it)
	HealthyPeople.Remove(Person);
	BittenPeople.Remove(Person);
	ZombiePeople.Remove(Person);

	// Recalculating stocks so numbers reflect remaining actors
	Susceptible = HealthyPeople.Num();
	Zombies = ZombiePeople.Num();
	Bitten = BittenPeople.Num();
}

// Function to read data from Unreal DataTable into the graphPts vector
void ASimulationController::ReadDataFromTableToVectors()
{
	graphPts.clear();

	TArray<FName> rowNames = PopulationDensityEffectTable->GetRowNames();

	for (int32 i = 0; i < rowNames.Num(); i++)
	{

		FPopulationDensityEffect* rowData =
			PopulationDensityEffectTable->FindRow<FPopulationDensityEffect>(rowNames[i], TEXT(""));

		if (rowData)
		{
			// Treat PopulationDensity as X and NormalPopulationDensity as Effect (Y)
			graphPts.emplace_back(rowData->PopulationDensity, rowData->NormalPopulationDensity);
		}
	}
}


float ASimulationController::GraphLookup(float X) const
{
	if (graphPts.empty())
	{
		return 1.f;
	}

	// Clamp if outside range
	if (X <= graphPts.front().first)
	{
		return graphPts.front().second;
	}
	if (X >= graphPts.back().first)
	{
		return graphPts.back().second;
	}

	// Linear interpolation between neighboring points
	for (size_t i = 1; i < graphPts.size(); ++i)
	{
		if (X <= graphPts[i].first)
		{
			const float X0 = graphPts[i - 1].first;
			const float X1 = graphPts[i].first;
			const float Y0 = graphPts[i - 1].second;
			const float Y1 = graphPts[i].second;

			const float T = (X - X0) / (X1 - X0);
			return Y0 + T * (Y1 - Y0);
		}
	}

	return graphPts.back().second;
}

float ASimulationController::ConveyorContent() const
{
	float Sum = 0.f;
	for (const FConveyorBatch& B : Conveyor)
	{
		Sum += B.AmountOfPeople;
	}
	return Sum;
}

// MAIN SIMULATION STEP
void ASimulationController::StepSimulation()
{
	Bitten = ConveyorContent();
	const float NonZombiePopulation = Bitten + Susceptible;

	const float PopulationDensity = (LandArea > 0.f)
		? NonZombiePopulation / LandArea
		: 0.f;

	const float X = (NormalPopulationDensity > 0.f)
		? PopulationDensity / NormalPopulationDensity
		: 0.f;

	const float PopDensityEffect = GraphLookup(X);
	const float BitesPerZombiePerDay = NormalNumberOfBites * PopDensityEffect;

	const float TotalBittenPerDay =
		FMath::RoundToFloat(Zombies * BitesPerZombiePerDay);

	const float Denom = FMath::Max(NonZombiePopulation, 1.f);
	const float BitesOnSusceptibleFloat =
		(Susceptible / Denom) * TotalBittenPerDay;

	float GettingBitten =
		FMath::RoundToFloat(BitesOnSusceptibleFloat);

	// Cannot bite more people than exist
	GettingBitten = FMath::Min(GettingBitten, FMath::FloorToFloat(Susceptible));

	LastBitesOnSusceptible = GettingBitten;

	//conveyor mechanics
	for (FConveyorBatch& B : Conveyor)
	{
		B.RemainingDays -= 1.f; // DT = 1 "day" per step
	}

	std::vector<FConveyorBatch> NextConveyor;
	NextConveyor.reserve(Conveyor.size());

	float RawOutflowPeople = 0.f;

	for (FConveyorBatch& B : Conveyor)
	{
		if (B.RemainingDays <= 0.f)
		{
			RawOutflowPeople += B.AmountOfPeople;
		}
		else
		{
			NextConveyor.push_back(B);
		}
	}

	Conveyor = std::move(NextConveyor);

	// capacity check for new inflow
	const float CurrentContent = ConveyorContent();
	const float FreeCap = FMath::Max(0.f, BittenCapacity - CurrentContent);
	const float InflowPeople = FMath::Max(0.f, FMath::Min(GettingBitten, FreeCap));

	if (InflowPeople > 0.f)
	{
		FConveyorBatch NewBatch;
		NewBatch.AmountOfPeople = InflowPeople;
		NewBatch.RemainingDays  = DaysToBecomeInfectedFromBite;
		Conveyor.push_back(NewBatch);
	}

	// outflow converted to zombies
	const float BecomingInfected = RawOutflowPeople * ConversionFromPeopleToZombies;

	const int32 NumNewBitten  = FMath::RoundToInt(InflowPeople);
	const int32 NumNewZombies = FMath::RoundToInt(BecomingInfected);
	
	int32 RemainingToBite = NumNewBitten;
	while (RemainingToBite > 0 && HealthyPeople.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, HealthyPeople.Num() - 1);
		APerson* Victim = HealthyPeople[Index];
		
		HealthyPeople.RemoveAtSwap(Index);
		BittenPeople.Add(Victim);
		
		Victim->SetState(EPersonState::Bitten);

		--RemainingToBite;
	}
	
	int32 RemainingToConvert = NumNewZombies;
	while (RemainingToConvert > 0 && BittenPeople.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, BittenPeople.Num() - 1);
		APerson* NewZombie = BittenPeople[Index];
		
		BittenPeople.RemoveAtSwap(Index);
		ZombiePeople.Add(NewZombie);
		
		NewZombie->SetState(EPersonState::Zombie);

		--RemainingToConvert;
	}
	
	// stock updates
	/*Susceptible = FMath::Max(0.f, Susceptible - GettingBitten);
	Zombies     = FMath::Max(0.f, Zombies + BecomingInfected);
	Bitten      = ConveyorContent(); */
	
	Susceptible = HealthyPeople.Num();
	Zombies = ZombiePeople.Num();
	Bitten = BittenPeople.Num();
}

 