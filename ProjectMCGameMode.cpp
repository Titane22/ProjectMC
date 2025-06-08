// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectMCGameMode.h"
#include "ProjectMCCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectMCGameMode::AProjectMCGameMode()
{
	// Find the pawn class in constructor (where ConstructorHelpers can be used)
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		// Set both DefaultPawnClass and DefaultPawnBlueprintClass to ensure consistency
		DefaultPawnClass = PlayerPawnBPClass.Class;
		DefaultPawnBlueprintClass = PlayerPawnBPClass.Class;
		FallbackPawnClass = PlayerPawnBPClass.Class;
	}
}

void AProjectMCGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Debug message to show which pawn class is being used
	if (DefaultPawnClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
			FString::Printf(TEXT("Using Pawn Class: %s"), *DefaultPawnClass->GetName()));
	}
}
