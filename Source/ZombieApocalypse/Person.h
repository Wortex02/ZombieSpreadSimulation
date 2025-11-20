// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Person.generated.h"

UENUM(BlueprintType)
enum class EPersonState : uint8
{
	Healthy UMETA(DisplayName="Healthy"),
	Bitten  UMETA(DisplayName="Bitten"),
	Zombie  UMETA(DisplayName="Zombie")
};

UCLASS()
class ZOMBIEAPOCALYPSE_API APerson : public ACharacter
{
	GENERATED_BODY()

public:
	APerson();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	EPersonState CurrentState = EPersonState::Healthy;

	// Called from C++ to change logic state
	UFUNCTION(BlueprintCallable, Category="State")
	void SetState(EPersonState NewState);

	// Blueprint gets this to play animations and switch mesh.
	UFUNCTION(BlueprintImplementableEvent, Category="State")
	void OnStateChanged(EPersonState NewState);
};

