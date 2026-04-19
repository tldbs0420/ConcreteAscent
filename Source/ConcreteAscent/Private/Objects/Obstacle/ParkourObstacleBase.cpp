// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Obstacle/ParkourObstacleBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AParkourObstacleBase::AParkourObstacleBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(SceneRoot);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ObstacleMesh->SetGenerateOverlapEvents(false);

	TraversalBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("TraversalBounds"));
	TraversalBounds->SetupAttachment(SceneRoot);
	TraversalBounds->SetBoxExtent(FVector(15.f, 50.f, 50.f));
	TraversalBounds->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TraversalBounds->SetCollisionObjectType(ECC_WorldStatic);
	TraversalBounds->SetCollisionResponseToAllChannels(ECR_Block);
	TraversalBounds->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AParkourObstacleBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// TODO: TraversalBound를 기준으로 폭과 높이 조정
}

