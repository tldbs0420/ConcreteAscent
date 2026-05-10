// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ParkourMotionData.h"

UPoseSearchDatabase* UParkourMotionData::GetLocomotionDatabase(EMovementMode State) const
{
	switch (State)
	{
	case EMovementMode::MOVE_Falling:
	case EMovementMode::MOVE_Flying:
		return JumpDatabase;
	case EMovementMode::MOVE_Walking:
	default:
		return MoveDatabase;
	}
}

UAnimMontage* UParkourMotionData::GetLedgeMoveMontage(float Direction) const
{
	return Direction < 0.f ? LedgeMoveLeftMontage : LedgeMoveRightMontage;
}
