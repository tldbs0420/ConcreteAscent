// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ConcreteAscentGameModeBase.h"
#include "Objects/Triggers/CheckpointActor.h"
#include "Player/ConcreteAscentCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Controller/ConcreteAscentPlayerController.h"


AConcreteAscentGameModeBase::AConcreteAscentGameModeBase()
{
}

void AConcreteAscentGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AConcreteAscentCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (PlayerCharacter)
		InitialRespawnTransform = PlayerCharacter->GetActorTransform();
}

void AConcreteAscentGameModeBase::SetCurrentCheckpoint(ACheckpointActor* NewCheckpoint)
{
	if (CurrentCheckpoint) CurrentCheckpoint->SetActivated(true);
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
	if (bGameCleared)
		return;

	bGameCleared = true;
	AConcreteAscentPlayerController* PlayerController = Cast<AConcreteAscentPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (!PlayerController)
		return;

	PlayerController->ShowGameClearUI();
}

FTransform AConcreteAscentGameModeBase::GetRespawnTransform()
{
	if (CurrentCheckpoint)
		return CurrentCheckpoint->GetRespawnTransform();

	return InitialRespawnTransform;
}