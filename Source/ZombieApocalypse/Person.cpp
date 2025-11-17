// Copyright University of Inland Norway


#include "Person.h"

APerson::APerson()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APerson::SetState(EPersonState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	OnStateChanged(NewState);
}


