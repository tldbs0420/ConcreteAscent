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
	
public:
	ALedgeObstacle();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float HorizontalLength = 200.f;

public:
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category = "Traversal")
	float GetHorizontalLength() const { return HorizontalLength; }
};
