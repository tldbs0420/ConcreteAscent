// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Types/ConcreteAscentTypes.h"
#include "ConcreteAscentCharacter.generated.h"

class UInputAction;
class UMotionWarpingComponent;
class UParkourTraversalComponent;

UCLASS()
class CONCRETEASCENT_API AConcreteAscentCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AConcreteAscentCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	EMovementState MovementState = EMovementState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascent|Movement")
	bool bCanMove = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bIsGrounded = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bIsHanging = false;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> JumpInputAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Traversal")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Traversal")
	TObjectPtr<UParkourTraversalComponent> ParkourTraversalComponent;

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	void Move(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Look(const FInputActionValue& InputValue);

	virtual void Jump() override;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void RespawnAt(const FTransform& RespawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetMovementState(EMovementState NewState);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void ApplyWarpTarget(const FName WarpTargetName, const FTransform& WarpTargetTransform);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	float PlayTraversalMontage(class UAnimMontage* Montage);

	UFUNCTION(BlueprintPure, Category = "Character")
	bool CanMove() const { return bCanMove; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsGrounded() const { return bIsGrounded; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsHanging() const { return bIsHanging; }

	UFUNCTION(BlueprintPure, Category = "Character")
	EMovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UParkourTraversalComponent* GetParkourTraversalComponent() const { return ParkourTraversalComponent; }

};
