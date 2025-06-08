// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerMech.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/TimelineComponent.h"

APlayerMech::APlayerMech()
{
	// Set default mech movement properties
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->JumpZVelocity = MechJumpVelocity;
		MovementComp->AirControl = MechAirControl;
		MovementComp->GravityScale = 1.5f; 
		MovementComp->BrakingFrictionFactor = 0.f;
		MovementComp->GroundFriction = 0.f;
		MovementComp->MaxWalkSpeed = NormalSpeed;
		MovementComp->bAllowPhysicsRotationDuringAnimRootMotion = true;
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 회전 속도 설정
	}
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false; 
	bUseControllerRotationRoll = false;

	DashCooldownTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DashCooldownTimeline"));
	TurnDashTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TurnDashTimeline"));
	VelocityDampingTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("VelocityDampingTimeline"));
}

void APlayerMech::BeginPlay()
{
	Super::BeginPlay();



	if (TurnDashCurve)
	{
		FOnTimelineFloat TurnDashFloat;
		TurnDashFloat.BindUFunction(this, FName("UpdateTurnDash"));
		TurnDashTimeline->AddInterpFloat(TurnDashCurve, TurnDashFloat);
	}

	if (VelocityDampingCurve)
	{
		FOnTimelineFloat VelocityDampingFloat;
		VelocityDampingFloat.BindUFunction(this, FName("UpdateVelocityDamping"));
		VelocityDampingTimeline->AddInterpFloat(VelocityDampingCurve, VelocityDampingFloat);

		FOnTimelineEvent VelocityDampingFinished;
		VelocityDampingFinished.BindUFunction(this, FName("FinishedVelocityDamping"));
		VelocityDampingTimeline->SetTimelineFinishedFunc(VelocityDampingFinished);
	}
}

void APlayerMech::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsBoosting && BoostEnergy > 0.f)
	{
		GetCharacterMovement()->MaxWalkSpeed = BoostSpeed;
		BoostEnergy -= BoostDepleteRate * DeltaTime;

		if (BoostEnergy <= 0.f)
		{
			BoostEnergy = 0.f;
			bIsBoosting = false;
		}
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		BoostEnergy += BoostRegenRate * DeltaTime;
		BoostEnergy = FMath::Min(BoostEnergy, MaxBoostEnergy);
	}
}

void APlayerMech::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Don't call Super first to avoid duplicate bindings
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Bind jump actions (from parent)
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Bind our custom Move and Look functions
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerMech::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerMech::Look);
		
		// Bind boost actions
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &APlayerMech::StartBoost);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &APlayerMech::EndBoost);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerMech::Dash);
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

void APlayerMech::StartBoost()
{
	if (BoostEnergy >= 0.f)
		bIsBoosting = true;
}

void APlayerMech::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	MoveValueX = MovementVector.X;
	MoveValueY = MovementVector.Y;
	
	// Update movement input state
	bIsMovementInput = !MovementVector.IsNearlyZero();
	
	// Update RelativeVelocity for dash system
	if (GetCharacterMovement())
	{
		FVector CurrentVelocity = GetCharacterMovement()->Velocity;
		RelativeVelocity = UKismetMathLibrary::LessLess_VectorRotator(CurrentVelocity, GetActorRotation());
	}
	
	// Call parent movement logic if needed
	if (GetWorld())
	{
		// Add movement input using the standard Unreal way
		if (Controller != nullptr)
		{
			// Find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			// Get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// Add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void APlayerMech::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X * 40.0f * GetWorld()->GetDeltaSeconds());
		AddControllerPitchInput(LookAxisVector.Y * 40.0f * GetWorld()->GetDeltaSeconds());

		// Store look values for dash system
		LookValueX = LookAxisVector.X * 0.4f;
		LookValueY = LookAxisVector.Y * 0.4f; // Fixed: was using X instead of Y
	}
}


void APlayerMech::EndBoost()
{
	bIsBoosting = false;
}

void APlayerMech::Dash()
{
	if (BoostEnergy <= 10.0f)
		return;
		
	if (!bIsMovementInput)
	{
		StartingControlRotation = GetControlRotation();
		TurnLookValueX = LookValueX;
		TurnDashTimeline->PlayFromStart();
	}
	else
	{
		// Check if input magnitude is sufficient (squared values comparison)
		float YSquared = FMath::Pow(MoveValueY, 2.0f);
		float XSquared = FMath::Pow(MoveValueX, 2.0f);
		
		if(YSquared >= XSquared)
		{
			// Forward/Backward dash - Y input is dominant
			ForwardBackDash();
		}
		else
		{
			// Side dash - X input is dominant  
			SideDash();
		}
	}
}

void APlayerMech::ForwardBackDash()
{
	if (!bIsValidForwardDash)
		return;

	BoostEnergy = BoostEnergy - 10.f;

	FVector XVector = GetActorRotation().Vector() * 15000.0f;
	float Direction = MoveValueY >= 0.0f ? 1.0f : -1.0f;
	LaunchCharacter(FVector(XVector.X * Direction, XVector.Y * Direction, 20.0f), false, false);

	// Clamp velocity like SideDash
	ClampCharacterVelocity();

	bIsValidForwardDash = false;
	bIsValidSideDash = false;

	// Use timer to reset dash flags after 1 second
	GetWorld()->GetTimerManager().ClearTimer(DashCooldownTimer);
	GetWorld()->GetTimerManager().SetTimer(DashCooldownTimer, [this]()
	{
		bIsValidForwardDash = true;
		bIsValidSideDash = true;
		bIsValidRightDash = true;
		bIsValidLeftDash = true;
	}, 1.0f, false);
}



void APlayerMech::UpdateTurnDash(float Value)
{
	float NewValue = Value * 90.0f * (TurnLookValueX >= 0.f ? 1.0f : -1.0f);
	FRotator NewRotation = FRotator(0.f, NewValue + StartingControlRotation.Yaw, 0.f);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(NewRotation);
	}
}

void APlayerMech::UpdateVelocityDamping(float Value)
{
	FVector CurVelocity = GetCharacterMovement()->Velocity;
	float NewVelocityX = FMath::Clamp(CurVelocity.X, -24000.0f, 24000.0f);
	float NewVelocityY = FMath::Clamp(CurVelocity.Y, -24000.0f, 24000.0f);
	float NewVelocityZ = FMath::Clamp(CurVelocity.Z, -24000.0f, 24000.0f);
	GetCharacterMovement()->Velocity = FMath::Lerp(CurVelocity, FVector(NewVelocityX, NewVelocityY, NewVelocityZ), Value);
}

void APlayerMech::FinishedVelocityDamping()
{
	// Velocity damping finished, but dash cooldown is handled by timer
}

void APlayerMech::SideDash()
{
	VelocityDampingTimeline->Stop();
	
	if (MoveValueX > 0.f) // Right dash
	{
		if (bIsValidRightDash)
		{
			if (RelativeVelocity.Y > 0.f)
			{
				bIsValidRightDash = false;
				bIsValidLeftDash = true;
				
				// Apply inverted Y velocity
				FVector InvertedYVelocity = FVector(RelativeVelocity.X, RelativeVelocity.Y * -1.f, RelativeVelocity.Z);
				GetCharacterMovement()->Velocity = UKismetMathLibrary::GreaterGreater_VectorRotator(InvertedYVelocity, GetActorRotation());
				
				// Perform dash with energy consumption
				PerformDashLaunch(12000.f, true);
			}
			else
			{
				// Update dash validity - right dash used, left dash available
				bIsValidRightDash = false;
				bIsValidLeftDash = true;
				
				// Perform dash without energy consumption
				PerformDashLaunch(15000.f, false);
			}
		}
	}
	else // Left dash
	{
		if (bIsValidLeftDash)
		{
			if (RelativeVelocity.Y > 0.f)
			{
				bIsValidRightDash = true;
				bIsValidLeftDash = false;
				
				// Apply inverted Y velocity
				FVector InvertedYVelocity = FVector(RelativeVelocity.X, RelativeVelocity.Y * -1.f, RelativeVelocity.Z);
				GetCharacterMovement()->Velocity = UKismetMathLibrary::GreaterGreater_VectorRotator(InvertedYVelocity, GetActorRotation());
				
				// Perform dash with energy consumption
				PerformDashLaunch(12000.f, true);
			}
			else
			{
				// Update dash validity - left dash used, right dash available
				bIsValidRightDash = true;
				bIsValidLeftDash = false;
				
				// Perform dash without energy consumption
				PerformDashLaunch(15000.f, false);
			}
		}
	}
}

void APlayerMech::PerformDashLaunch(float VelocityMultiplier, bool bConsumeEnergy)
{
	if (bConsumeEnergy)
	{
		BoostEnergy -= 10.0f;
	}
	
	FRotator ComposedRotator = UKismetMathLibrary::ComposeRotators(GetActorRotation(), FRotator(0.f, 90.f, 0.f));
	float DirectionMultiplier = MoveValueX >= 0.0f ? 1.0f : -1.0f;
	float LaunchVelocityX = ComposedRotator.Pitch * VelocityMultiplier * DirectionMultiplier;
	float LaunchVelocityY = ComposedRotator.Yaw * VelocityMultiplier * DirectionMultiplier;
	
	LaunchCharacter(FVector(LaunchVelocityX, LaunchVelocityY, 20.0f), false, false);
	
	ClampCharacterVelocity();
	SetupPostDashState();
}

void APlayerMech::ClampCharacterVelocity()
{
	GetCharacterMovement()->Velocity = FVector(
		FMath::Clamp(GetCharacterMovement()->Velocity.X, -2000.0f, 2000.0f),
		FMath::Clamp(GetCharacterMovement()->Velocity.Y, -2000.0f, 2000.0f),
		FMath::Clamp(GetCharacterMovement()->Velocity.Z, -2000.0f, 2000.0f)
	);
}

void APlayerMech::SetupPostDashState()
{
	bIsValidForwardDash = false;
	bIsValidSideDash = false;
	
	// Use timer to reset dash flags after 1 second
	GetWorld()->GetTimerManager().ClearTimer(DashCooldownTimer);
	GetWorld()->GetTimerManager().SetTimer(DashCooldownTimer, [this]()
	{
		bIsValidForwardDash = true;
		bIsValidSideDash = true;
		bIsValidRightDash = true;
		bIsValidLeftDash = true;
	}, 1.0f, false);
}
