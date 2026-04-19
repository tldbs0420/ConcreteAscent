// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/ConcreteAscentTypes.h"
#include "ParkourMotionData.generated.h"

class UAnimMontage;
class UPoseSearchDatabase;

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API UParkourMotionData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
	TObjectPtr<UPoseSearchDatabase> MoveDatabase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
	TObjectPtr<UPoseSearchDatabase> JumpDatabase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeGrabMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeMoveLeftMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeMoveRightMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeClimbUpMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeDropMontage;

	UFUNCTION(BlueprintPure, Category = "Motion Matching")
	UPoseSearchDatabase* GetLocomotionDatabase(EMovementState State) const;

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UAnimMontage* GetLedgeGrabMontage() const { return LedgeGrabMontage; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UAnimMontage* GetLedgeMoveMontage(float Direction) const;

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UAnimMontage* GetLedgeClimbUpMontage() const { return LedgeClimbUpMontage; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UAnimMontage* GetLedgeDropMontage() const { return LedgeDropMontage; }
};
