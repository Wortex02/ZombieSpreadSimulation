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

    // Multiple lines for better organization
    FString stepMessage = FString::Printf(TEXT("Day: %d"), SimulationController->TimeStepsFinished);
    FString humansMessage = FString::Printf(TEXT("Humans: %d"), (int)SimulationController->Susceptible);
    FString bittenMessage = FString::Printf(TEXT("Bitten: %d"), (int)SimulationController->Bitten);
    FString zombiesMessage = FString::Printf(TEXT("Zombies: %d"), (int)SimulationController->Zombies);
    FString innocentKillsMessage = FString::Printf(TEXT("Innocents killed: %d"), (int)SimulationController->InnocentKills);
    FString infectedKillsMessage = FString::Printf(TEXT("Infected killed: %d"), (int)SimulationController->InfectedKills);
    FString zombieKillsMessage = FString::Printf(TEXT("Zombies killed: %d"), (int)SimulationController->ZombieKills);

    DrawText(stepMessage, textColor, screenPosition.X - 45.0f, screenPosition.Y, nullptr, textScale, true);
    DrawText(humansMessage, textColor, screenPosition.X - 45.0f, screenPosition.Y + 15.0f, nullptr, textScale, true);
    DrawText(bittenMessage, textColor, screenPosition.X - 45.0f, screenPosition.Y + 30.0f, nullptr, textScale, true);
    DrawText(zombiesMessage, textColor, screenPosition.X - 45.0f, screenPosition.Y + 45.0f, nullptr, textScale, true);
    DrawText(innocentKillsMessage, textColor, screenPosition.X - 45.0f, screenPosition.Y + 60.0f, nullptr, textScale, true);
    DrawText(infectedKillsMessage, textColor, screenPosition.X - 45.0f, screenPosition.Y + 75.0f, nullptr, textScale, true);
    DrawText(zombieKillsMessage, textColor, screenPosition.X - 45.0f, screenPosition.Y + 90.0f, nullptr, textScale, true);
}
