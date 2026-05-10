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
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class CONCRETEASCENT_API AConcreteAscentCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AConcreteAscentCharacter();

protected:
	// ACharacter Override
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	EGait Gait = EGait::Walk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascent|Movement")
	bool bCanMove = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bWalk = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bSprint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bIsGrounded = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bIsHanging = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bJustLanded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float WalkMaxSpeed = 200;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float RunMaxSpeed = 500;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float SprintMaxSpeed = 700;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float LastLandingVerticalSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> JumpInputAction;
	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> WalkInputAction;
	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> SprintInputAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Traversal")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Traversal")
	TObjectPtr<UParkourTraversalComponent> ParkourTraversalComponent;

	UFUNCTION(BlueprintCallable, Category = "Ascent|Movement")
	void UpdateGait();

	FTimerHandle JustLandedTimerHandle;
	void ClearJustLanded();

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	void Move(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Look(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void ToggleWalk(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void StartSprint(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void StopSprint(const FInputActionValue& InputValue);

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
	bool IsWalking() const { return bWalk; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsSprinting() const { return bSprint; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsJustLanded() const { return bJustLanded; }

	UFUNCTION(BlueprintPure, Category = "Character")
	float GetLastLandingVerticalSpeed() const { return LastLandingVerticalSpeed; }

	UFUNCTION(BlueprintPure, Category = "Character")
	EGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UParkourTraversalComponent* GetParkourTraversalComponent() const { return ParkourTraversalComponent; }

};
