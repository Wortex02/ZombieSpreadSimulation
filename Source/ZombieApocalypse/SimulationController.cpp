// Copyright University of Inland Norway

#include "SimulationController.h"
#include  <cmath>

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
}

void ASimulationController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AccumulatedTime += DeltaTime;

    // If Unreal timestep is reached
    if (AccumulatedTime >= SimulationStepTime)
    {
        AccumulatedTime = 0.f; // Reset accumulator
        UE_LOG(LogTemp, Log, TEXT("SimulationStep"));
    }  
}

// Function to read data from Unreal DataTable into the graphPts vector
void ASimulationController::ReadDataFromTableToVectors()
{
    if (bShouldDebug)
       UE_LOG(LogTemp, Log, TEXT("Read Data From Table To Vectors"))

    TArray<FName> rowNames = PopulationDensityEffectTable->GetRowNames();

    for (int i{0}; i < rowNames.Num(); i++)
    {
       if (bShouldDebug)
          UE_LOG(LogTemp, Log, TEXT("Reading table row index: %d"), i)

       FPopulationDensityEffect* rowData = PopulationDensityEffectTable->FindRow<FPopulationDensityEffect>(rowNames[i], TEXT(""));
       if (rowData)
       {
           graphPts.emplace_back(rowData->PopulationDensity, rowData->NormalPopulationDensity);

           if (bShouldDebug)
           {
               std::pair<float, float> lastPair = graphPts.back();
               UE_LOG(LogTemp, Warning, TEXT("Reading table row: %d, pair: (%f, %f)"), i, lastPair.first, lastPair.second);
           }
       }
    }
}
 