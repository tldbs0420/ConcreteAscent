// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animation/ConcreteAscentAnimInstance.h"
#include "Player/ConcreteAscentCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Data/ParkourMotionData.h"
#include "Player/Components/ParkourTraversalComponent.h"

void UConcreteAscentAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AConcreteAscentCharacter>(TryGetPawnOwner());
}

void UConcreteAscentAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
		OwnerCharacter = Cast<AConcreteAscentCharacter>(TryGetPawnOwner());

	UpdateFromCharacter();
}

void UConcreteAscentAnimInstance::UpdateFromCharacter()
{
	if (!OwnerCharacter)
		return;

	const FVector Velocity = OwnerCharacter->GetVelocity();
	Speed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
	bIsInAir = OwnerCharacter->GetCharacterMovement() ? OwnerCharacter->GetCharacterMovement()->IsFalling() : false;
	MovementState = OwnerCharacter->GetMovementState();
}

UPoseSearchDatabase* UConcreteAscentAnimInstance::SelectLocomotionDatabase() const
{
	return MotionData ? MotionData->GetLocomotionDatabase(MovementState) : nullptr;
}