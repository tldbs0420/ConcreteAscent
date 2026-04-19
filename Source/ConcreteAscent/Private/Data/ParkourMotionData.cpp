// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ParkourMotionData.h"

UPoseSearchDatabase* UParkourMotionData::GetLocomotionDatabase(EMovementState State) const
{
	switch (State)
	{
	case EMovementState::Jumping:
	case EMovementState::Falling:
		return JumpDatabase;
	case EMovementState::Idle:
	case EMovementState::Walking:
	case EMovementState::Running:
	default:
		return MoveDatabase;
	}
}

UAnimMontage* UParkourMotionData::GetLedgeMoveMontage(float Direction) const
{
	return Direction < 0.f ? LedgeMoveLeftMontage : LedgeMoveRightMontage;
}
