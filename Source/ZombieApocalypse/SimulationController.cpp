// Copyright University of Inland Norway

#include "SimulationController.h"
#include "Engine/DataTable.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include <cmath>

ASimulationController::ASimulationController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASimulationController::BeginPlay()
{
	Super::BeginPlay();

	// Checking if the DataTable is assigned
	if (!PopulationDensityEffectTable)
	{
		UE_LOG(LogTemp, Error, TEXT("PopulationDensityEffectTable is not assigned!"));
	}
	else
	{
		// Table found, read data into vector
		ReadDataFromTableToVectors();
	}

	// Initial stocks if you want to enforce them here
	// Susceptible = 100.f;
	// Zombies     = 1.f;
	// Bitten will be derived from Conveyor, so start empty
	Conveyor.clear();
	Bitten = 0.f;
}

void ASimulationController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AccumulatedTime += DeltaTime;

	// If Unreal timestep is reached
	if (AccumulatedTime >= SimulationStepTime)
	{
		AccumulatedTime -= SimulationStepTime; // keep residual
		StepSimulation();
		TimeStepsFinished++;

		if (bShouldDebug)
		{
			UE_LOG(LogTemp, Log,
				TEXT("Day %d -> Susceptible: %.2f, Bitten: %.2f, Zombies: %.2f, LastBitesOnSusceptible: %.0f"),
				TimeStepsFinished,
				Susceptible,
				Bitten,
				Zombies,
				LastBitesOnSusceptible);
		}
	}
}

// Function to read data from Unreal DataTable into the graphPts vector
void ASimulationController::ReadDataFromTableToVectors()
{
	if (bShouldDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Read Data From Table To Vectors"));
	}

	graphPts.clear();

	TArray<FName> rowNames = PopulationDensityEffectTable->GetRowNames();

	for (int32 i = 0; i < rowNames.Num(); i++)
	{
		if (bShouldDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Reading table row index: %d"), i);
		}

		FPopulationDensityEffect* rowData =
			PopulationDensityEffectTable->FindRow<FPopulationDensityEffect>(rowNames[i], TEXT(""));

		if (rowData)
		{
			// Treat PopulationDensity as X and NormalPopulationDensity as Effect (Y)
			graphPts.emplace_back(rowData->PopulationDensity, rowData->NormalPopulationDensity);

			if (bShouldDebug)
			{
				const std::pair<float, float>& lastPair = graphPts.back();
				UE_LOG(LogTemp, Warning,
					TEXT("Row %d: pair: (X=%f, Y=%f)"),
					i, lastPair.first, lastPair.second);
			}
		}
	}
}

// ---------- HELPER FUNCTIONS ----------

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

// ---------- MAIN SIMULATION STEP ----------

void ASimulationController::StepSimulation()
{
	// ----- auxiliaries -----
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

	// ----- conveyor mechanics -----
	// 1) progress existing batches
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

	// 2) capacity check for new inflow
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

	// 3) outflow converted to zombies
	const float BecomingInfected = RawOutflowPeople * ConversionFromPeopleToZombies;

	// ----- stock updates -----
	Susceptible = FMath::Max(0.f, Susceptible - GettingBitten);
	Zombies     = FMath::Max(0.f, Zombies + BecomingInfected);
	Bitten      = ConveyorContent(); // refresh from conveyor
}

 