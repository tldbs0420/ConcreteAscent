// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Obstacle/ParkourObstacleBase.h"
#include "LedgeObstacle.generated.h"

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API ALedgeObstacle : public AParkourObstacleBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float HorizontalLength = 200.f;

public:
	UFUNCTION(BlueprintPure, Category = "Traversal")
	float GetHorizontalLength() const { return HorizontalLength; }

	UFUNCTION(BlueprintCallable, Category = "Traversal|Ledge")
	bool GetLedgeMoveSegment(const FVector& FrontNormal, FVector& OutCenter, FVector& OutRight, float& OutHalfLength) const;
};
