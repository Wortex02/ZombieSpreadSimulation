// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include <vector>
#include "SimulationController.generated.h"


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



UCLASS()
class ZOMBIEAPOCALYPSE_API ASimulationController : public AActor
{
	GENERATED_BODY()
	
public:	
	ASimulationController();

	// Runs one simulation step each Tick 
	virtual void Tick(float DeltaTime) override;

	// Function to read data from Unreal DataTable into the graphPts vector
	void ReadDataFromTableToVectors();


	// Unreal Lookup table for population density effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	class UDataTable* PopulationDensityEffectTable{ nullptr };

	// How long each time step is in Unreal, in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	float SimulationStepTime{ 1.f };

	// Turn on/off debug printing to the Output Log
	UPROPERTY(EditAnywhere, Category = "Simulation Variables")
	bool bShouldDebug{ false };


	// Stocks (initial)

	//Susceptible (People) - choose a number that has a clean sqrt!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	float Susceptible{ 100.f };
	// Zombies = patient_zero
	float Zombies{ 1.f };		
	// Just to check if we are correctly updating stocks - used in SimulationHUD
	float Bitten{ 0.f };          // s Bitten -> in BittenArraySize


	
	// GRAPH points: population_density_effect_on_zombie_bites
	// Values read in from Unreal DataTable for more flexibility
	std::vector<std::pair<float, float>> graphPts;
		//= {
		//{0.000f, 0.014f}, {0.200f, 0.041f}, {0.400f, 0.101f}, {0.600f, 0.189f}, {0.800f, 0.433f},
		//{1.000f, 1.000f}, {1.200f, 1.217f}, {1.400f, 1.282f}, {1.600f, 1.300f}, {1.800f, 1.300f},
		//{2.000f, 1.300f}
		//};

	// Time accumulator for simulation steps, used in Tick function
	float AccumulatedTime{ 0.f };

	// Number of time steps completed - to keep track and compare to Stella
	int TimeStepsFinished{ 0 };



protected:
	virtual void BeginPlay() override;

};
