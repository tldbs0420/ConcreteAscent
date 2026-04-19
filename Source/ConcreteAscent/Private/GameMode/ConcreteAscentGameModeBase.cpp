// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ConcreteAscentGameModeBase.h"
#include "Objects/Triggers/CheckpointActor.h"
#include "Player/ConcreteAscentCharacter.h"
#include "Kismet/GameplayStatics.h"


AConcreteAscentGameModeBase::AConcreteAscentGameModeBase()
{
}

void AConcreteAscentGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AConcreteAscentCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void AConcreteAscentGameModeBase::SetCurrentCheckpoint(ACheckpointActor* NewCheckpoint)
{
	CurrentCheckpoint = NewCheckpoint;
}

void AConcreteAscentGameModeBase::HandleRespawnRequest()
{
	if (!PlayerCharacter || !CanRespawn())
		return;

	PlayerCharacter->RespawnAt(GetRespawnTransform());
}

void AConcreteAscentGameModeBase::HandleGoalReached()
{
	if (CanClear())
		HandleGameClear();
}

void AConcreteAscentGameModeBase::HandleGameClear()
{
	bGameCleared = true;

	if (PlayerCharacter)
		PlayerCharacter->SetMovementState(EMovementState::Cleared);

	// TODO: UI로 종료 화면 보여주기
}

FTransform AConcreteAscentGameModeBase::GetRespawnTransform()
{
	if (CurrentCheckpoint)
		return CurrentCheckpoint->GetRespawnTransform();

	return FTransform::Identity;
}