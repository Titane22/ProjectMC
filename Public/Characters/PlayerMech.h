// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../ProjectMCCharacter.h"
#include "PlayerMech.generated.h"

/**
 * Player Mech Character with customizable jump behavior
 */
UCLASS()
class PROJECTMC_API APlayerMech : public AProjectMCCharacter
{
	GENERATED_BODY()
	
public:
	APlayerMech();

protected:
	// Override jump functions to customize mech jump behavior
	
	/** Called when jump input is started - can be overridden for custom jump behavior */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mech Movement")
	void OnJumpStart();
	
	/** Called when jump input is completed - can be overridden for custom jump stop behavior */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mech Movement")
	void OnJumpStop();
	
	/** Override Jump function for custom mech jump logic */
	virtual void Jump() override;
	
	/** Override StopJumping function for custom mech jump stop logic */
	virtual void StopJumping() override;
	
	/** Customizable jump parameters for mech */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Movement")
	float MechJumpVelocity = 1200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Movement")
	float MechAirControl = 0.8f;
};
