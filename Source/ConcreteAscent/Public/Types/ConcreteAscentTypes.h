// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConcreteAscentTypes.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	OnGround UMETA(DisplayName = "OnGround"),
	InAir UMETA(DisplayName = "InAir"),
	Hanging UMETA(DisplayName = "Hanging"),
};

UENUM(BlueprintType)
enum class EMoveMode : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Move UMETA(DisplayName = "Move"),
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk UMETA(DisplayName = "Walk"),
	Run UMETA(DisplayName = "Run"),
	Sprint UMETA(DisplayName = "Sprint"),
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
