// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Obstacle/LedgeObstacle.h"

ALedgeObstacle::ALedgeObstacle()
{
}

void ALedgeObstacle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// TODO: 좌우길이까지 계산하기
}