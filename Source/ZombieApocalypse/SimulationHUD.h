// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SimulationHUD.generated.h"

UCLASS()
class ZOMBIEAPOCALYPSE_API ASimulationHUD : public AHUD
{
	GENERATED_BODY()


public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

private:
	class ASimulationController* SimulationController{ nullptr };


};
