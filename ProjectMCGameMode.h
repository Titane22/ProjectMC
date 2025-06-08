// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectMCGameMode.generated.h"

UCLASS(minimalapi)
class AProjectMCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProjectMCGameMode();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/** Default pawn class that can be set in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Mode Settings", meta = (DisplayName = "Default Pawn Class"))
	TSubclassOf<class APawn> DefaultPawnBlueprintClass;

private:
	/** Fallback pawn class found in constructor */
	TSubclassOf<class APawn> FallbackPawnClass;
};



