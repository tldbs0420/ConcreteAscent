// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Types/ConcreteAscentTypes.h"
#include "ConcreteAscentAnimInstance.generated.h"

class AConcreteAscentCharacter;
class UParkourMotionData;
class UPoseSearchDatabase;

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API UConcreteAscentAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<AConcreteAscentCharacter> OwnerCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UParkourMotionData> MotionData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	float Speed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	float Direction = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsInAir = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	EMovementState MovementState = EMovementState::Idle;

public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateFromCharacter();

	UFUNCTION(BlueprintPure, Category = "Animation")
	UPoseSearchDatabase* SelectLocomotionDatabase() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	AConcreteAscentCharacter* GetOwnerCharacter() const { return OwnerCharacter; }
};
