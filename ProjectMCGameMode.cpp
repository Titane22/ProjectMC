// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectMCGameMode.h"
#include "ProjectMCCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectMCGameMode::AProjectMCGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
