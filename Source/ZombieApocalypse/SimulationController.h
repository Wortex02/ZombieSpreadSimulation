// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include <vector>
#include "SimulationController.generated.h"

class APerson;
// Struct for the Unreal DataTable
USTRUCT(BlueprintType)
struct FPopulationDensityEffect : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PopulationDensity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NormalPopulationDensity;
};



struct FConveyorBatch
{
	float AmountOfPeople = 0.f;
	float RemainingDays  = 0.f;
};


UCLASS()
class ZOMBIEAPOCALYPSE_API ASimulationController : public AActor
{
	GENERATED_BODY()
	
public:	
	ASimulationController();

	// Runs one simulation step every SimulationStepTime seconds
	virtual void Tick(float DeltaTime) override;

	// Read DataTable into graphPts
	void ReadDataFromTableToVectors();

	// Unreal DataTable for population density effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	class UDataTable* PopulationDensityEffectTable{ nullptr };

	// How long each time step is in Unreal, in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	float SimulationStepTime{ 1.f };


	// SIMULATION PARAMETERS
	//Susceptible (People) - choose a number that has a clean sqrt!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Stocks")
	float Susceptible{ 100.f };

	// Zombies = patient_zero
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Stocks")
	float Zombies{ 1.f };
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation Variables|Stocks")
	float Bitten{ 0.f };   // Sum of conveyor contents

	// Days it takes from bite to becoming a zombie
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Params")
	float DaysToBecomeInfectedFromBite{ 15.f };

	// Maximum number of people in the "Bitten" conveyor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Params")
	float BittenCapacity{ 100.f };

	// Conversion rate from bitten people to zombies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Params")
	float ConversionFromPeopleToZombies{ 1.f };

	// Normal number of bites per zombie per day
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Params")
	float NormalNumberOfBites{ 1.f }; // people/zombie/day

	// Land area where people live (m^2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Params")
	float LandArea{ 1000.f };         // m2

	// Normal population density (people/m^2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables|Params")
	float NormalPopulationDensity{ 0.1f }; // people/m2


	// GRAPH points: population_density_effect_on_zombie_bites
	// Values read in from Unreal DataTable for more flexibility
	std::vector<std::pair<float, float>> graphPts;

	// Time accumulator for simulation steps, used in Tick function
	float AccumulatedTime{ 0.f };

	// Number of time steps completed - used in HUD
	int TimeStepsFinished{ 0 };

	// Last number of bites on susceptible in this step
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation Variables|Debug")
	float LastBitesOnSusceptible{ 0.f };

	// Grid
	UPROPERTY(EditAnywhere, Category="Grid")
	TSubclassOf<APerson> PersonClass;

	// Grid dimensions
	UPROPERTY(EditAnywhere, Category="Grid")
	int32 GridSizeX{10};

	UPROPERTY(EditAnywhere, Category="Grid")
	int32 GridSizeY{10};

	// Distance between people in the grid (cm)
	UPROPERTY(EditAnywhere, Category="Grid")
	float CellSpacing{200.f};

	// Where the grid starts
	UPROPERTY(EditAnywhere, Category="Griid")
	FVector GridOrigin{ FVector(1000.f, 0.f, 0.f) };

	// Arrays to track people by state
	UPROPERTY()
	TArray<APerson*> HealthyPeople;

	UPROPERTY()
	TArray<APerson*> BittenPeople;

	UPROPERTY()
	TArray<APerson*> ZombiePeople;

	UFUNCTION()
	void OnPersonDestroyed(AActor* DestroyedActor);

	// Spawn visual grid + patient zero
	void SpawnGrid();
protected:
	virtual void BeginPlay() override;

private:
	// Conveyor storage
	std::vector<FConveyorBatch> Conveyor;

	// interpolates in graphPts
	float GraphLookup(float X) const;

	// sums all AmountOfPeople in Conveyor
	float ConveyorContent() const;

	// One simulation "day" step (called every SimulationStepTime seconds)
	void StepSimulation();
};
