// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerMech.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerMech::APlayerMech()
{
	// Set default mech movement properties
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->JumpZVelocity = MechJumpVelocity;
		MovementComp->AirControl = MechAirControl;
		MovementComp->GravityScale = 1.5f; // Heavier feel for mech
	}
}

void APlayerMech::Jump()
{
	// Call Blueprint implementable event first
	OnJumpStart();
	
	// Apply custom mech jump logic
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		// Update jump velocity with current mech settings
		MovementComp->JumpZVelocity = MechJumpVelocity;
		MovementComp->AirControl = MechAirControl;
	}
	
	// Call parent Jump function
	Super::Jump();
}

void APlayerMech::StopJumping()
{
	// Call Blueprint implementable event first
	OnJumpStop();
	
	// Call parent StopJumping function
	Super::StopJumping();
}

