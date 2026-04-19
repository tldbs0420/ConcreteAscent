// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConcreteAscentTypes.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Jumping UMETA(DisplayName = "Jumping"),
	Falling UMETA(DisplayName = "Falling"),
	Hanging UMETA(DisplayName = "Hanging"),
	Climbing UMETA(DisplayName = "Climbing"),
	Cleared UMETA(DisplayName = "Cleared")
};

UENUM(BlueprintType)
enum class ETraversalAction : uint8
{
	None UMETA(DisplayName = "None"),
	Vault UMETA(DisplayName = "Vault"),
	Hurdle UMETA(DisplayName = "Hurdle"),
	Mantle UMETA(DisplayName = "Mantle"),
	LedgeGrab UMETA(DisplayName = "LedgeGrab"),
	LedgeClimbUp UMETA(DisplayName = "LedgeClimbUp"),
	LedgeMoveLeft UMETA(DisplayName = "LedgeMoveLeft"),
	LedgeMoveRight UMETA(DisplayName = "LedgeMoveRight"),
	LedgeDrop UMETA(DisplayName = "LedgeDrop")
};

USTRUCT(BlueprintType)
struct CONCRETEASCENT_API FTraversalChooserInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	EMovementState MovementState = EMovementState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	ETraversalAction TraversalAction = ETraversalAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleThickness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackFloor = false;
};
