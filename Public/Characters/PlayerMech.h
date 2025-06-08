// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../ProjectMCCharacter.h"
#include "PlayerMech.generated.h"

class UTimelineComponent;

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
	/** Called when jump input is started - can be overridden for custom jump behavior */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mech Movement")
	void OnJumpStart();
	
	/** Called when jump input is completed - can be overridden for custom jump stop behavior */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mech Movement")
	void OnJumpStop();
	
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	virtual void BeginPlay() override;

	/** Override Jump function for custom mech jump logic */
	virtual void Jump() override;
	
	/** Override StopJumping function for custom mech jump stop logic */
	virtual void StopJumping() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void StartBoost();

	void EndBoost();

	void Dash();

	void SideDash();

	void ForwardBackDash();

	UFUNCTION()
	void UpdateTurnDash(float Value);

	UFUNCTION()
	void UpdateVelocityDamping(float Value);

	UFUNCTION()
	void FinishedVelocityDamping();

private:
	/** Helper function to perform dash launch with specified velocity multiplier */
	void PerformDashLaunch(float VelocityMultiplier, bool bConsumeEnergy = false);
	
	/** Helper function to clamp character velocity */
	void ClampCharacterVelocity();
	
	/** Helper function to setup dash state after launch */
	void SetupPostDashState();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BoostAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Movement")
	bool bIsMovementInput = false;

	/** Customizable jump parameters for mech */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Movement")
	float MechJumpVelocity = 1200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Movement")
	float MechAirControl = 0.8f;

	UPROPERTY(BlueprintReadOnly, Category = "Boost")
	bool bIsBoosting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float BoostSpeed = 1800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float NormalSpeed = 550.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Boost")
	float BoostEnergy = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float MaxBoostEnergy = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float BoostDepleteRate = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float BoostRegenRate = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bIsValidLeftDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bIsValidRightDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bIsValidForwardDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bIsValidSideDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	FVector RelativeVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float MoveValueX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float MoveValueY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	UTimelineComponent* DashCooldownTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	UTimelineComponent* TurnDashTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	UTimelineComponent* VelocityDampingTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	UCurveFloat* DashCooldownCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	UCurveFloat* TurnDashCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	UCurveFloat* VelocityDampingCurve;

	FRotator StartingControlRotation;

	float TurnLookValueX;

	float LookValueX;

	float LookValueY;

	FTimerHandle DashCooldownTimer;
};
