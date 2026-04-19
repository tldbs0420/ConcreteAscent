// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ConcreteAscentCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "Player/Components/ParkourTraversalComponent.h"
#include "MotionWarpingComponent.h"

AConcreteAscentCharacter::AConcreteAscentCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	ParkourTraversalComponent = CreateDefaultSubobject<UParkourTraversalComponent>(TEXT("ParkourTraversalComponent"));
}

void AConcreteAscentCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	bIsGrounded = GetCharacterMovement() ? GetCharacterMovement()->IsMovingOnGround() : true;
}

void AConcreteAscentCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AConcreteAscentCharacter::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AConcreteAscentCharacter::Move);
		}

		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AConcreteAscentCharacter::Look);

		if (JumpInputAction)
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::Jump);
	}
}

void AConcreteAscentCharacter::Move(const FInputActionValue& InputValue)
{
	// TODO: 이동 구현
}

void AConcreteAscentCharacter::Look(const FInputActionValue& InputValue)
{
	const FVector2D LookAxis = InputValue.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void AConcreteAscentCharacter::Jump()
{
	// TODO: 파쿠르 관련 동작 처리
	// 파쿠르 미수행 시 점프 동작

	SetMovementState(EMovementState::Jumping);
}

void AConcreteAscentCharacter::RespawnAt(const FTransform& RespawnTransform)
{
	SetActorLocationAndRotation(RespawnTransform.GetLocation(), RespawnTransform.GetRotation().Rotator(), false, nullptr, ETeleportType::TeleportPhysics);

	if (UCharacterMovementComponent* AscentCharacterMovement = GetCharacterMovement())
	{
		AscentCharacterMovement->StopMovementImmediately();
		AscentCharacterMovement->Velocity = FVector::ZeroVector;
	}

	bCanMove = true;
	bIsGrounded = true;
	bIsHanging = false;
	SetMovementState(EMovementState::Idle);
}

void AConcreteAscentCharacter::SetMovementState(EMovementState NewState)
{
	// TODO: 플레이어 상태에 따라 bool 갱신
	return;
}

void AConcreteAscentCharacter::ApplyWarpTarget(const FName WarpTargetName, const FTransform& WarpTargetTransform)
{
	if (MotionWarpingComponent)
	{
		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(WarpTargetName, WarpTargetTransform);
	}
}

float AConcreteAscentCharacter::PlayTraversalMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return 0.f;
	}

	return PlayAnimMontage(Montage);
}