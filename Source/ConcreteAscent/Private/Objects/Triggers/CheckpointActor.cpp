// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Triggers/CheckpointActor.h"
#include "Components/SceneComponent.h"

ACheckpointActor::ACheckpointActor()
{
	RespawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnPoint"));
	RespawnPoint->SetupAttachment(RootComponent);
	RespawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	RespawnPoint->SetRelativeRotation(FRotator::ZeroRotator);
}

FTransform ACheckpointActor::GetRespawnTransform() const
{
	return RespawnPoint ? RespawnPoint->GetComponentTransform() : GetActorTransform();
}

void ACheckpointActor::SetActivated(bool bInActivated)
{
	bIsEnabled = bInActivated;
}

void ACheckpointActor::OnPlayerEntered(AConcreteAscentCharacter* Character)
{
	if (!Character || !CachedGameMode)
		return;

	bIsEnabled = false;

	// TODO: GameMode에 체크포인트 등록
	// TODO: 체크포인트 활성화 VFX/SFX/UI 처리
}