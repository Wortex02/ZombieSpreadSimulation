// Copyright University of Inland Norway


#include "SimulationHUD.h"
#include "Kismet/GameplayStatics.h"
#include "SimulationController.h"

void ASimulationHUD::BeginPlay()
{
	Super::BeginPlay();
    SimulationController = Cast<ASimulationController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulationController::StaticClass()));

    if (!SimulationController)
    {
        UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: SimulationController not found!"));
    }
    UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: SimulationController found!"));
}

void ASimulationHUD::DrawHUD()
{
	Super::DrawHUD();

    FVector2D screenPosition(50.0f, 50.0f); // X, Y position on screen
    FLinearColor textColor = FLinearColor::White;
    float textScale = 2.f;

    //FString message = FString::Printf(TEXT("Day: %d, Humans: %f, Bitten: %f, Zombies: %f"),
		//SimulationController->timeStepsFinished,
        //SimulationController->Susceptible,
        //SimulationController->Bitten,
        //SimulationController->Zombies);

    //DrawText(message, textColor, screenPosition.X, screenPosition.Y, nullptr, textScale, true);
	
    FString stepMessage = FString::Printf(TEXT("Day: %d"), SimulationController->TimeStepsFinished);
    FString humansMessage = FString::Printf(TEXT("Humans: %d"), (int)SimulationController->Susceptible);
    FString bittenMessage = FString::Printf(TEXT("Bitten: %d"), (int)SimulationController->Bitten);
    FString zombiesMessage = FString::Printf(TEXT("Zombies: %d"), (int)SimulationController->Zombies);

    DrawText(stepMessage, textColor, screenPosition.X, screenPosition.Y, nullptr, textScale, true);
    DrawText(humansMessage, textColor, screenPosition.X, screenPosition.Y + 15.0f, nullptr, textScale, true);
    DrawText(bittenMessage, textColor, screenPosition.X, screenPosition.Y + 30.0f, nullptr, textScale, true);
    DrawText(zombiesMessage, textColor, screenPosition.X, screenPosition.Y + 45.0f, nullptr, textScale, true);
}
