// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ConcreteAscentGameModeBase.generated.h"

class ACheckpointActor;
class AConcreteAscentCharacter;
class UUserWidget;

/**
 * 
 */
UCLASS(Abstract)
class CONCRETEASCENT_API AConcreteAscentGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AConcreteAscentGameModeBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Game")
	TObjectPtr<AConcreteAscentCharacter> PlayerCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> InGameHUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameClearUIClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Game")
	TObjectPtr<ACheckpointActor> CurrentCheckpoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	FTransform InitialRespawnTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	bool bGameCleared = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetCurrentCheckpoint(ACheckpointActor* NewCheckpoint);

	UFUNCTION(BlueprintCallable, Category = "Game")
	virtual void HandleRespawnRequest();

	UFUNCTION(BlueprintCallable, Category = "Game")
	virtual void HandleGoalReached();

	UFUNCTION(BlueprintCallable, Category = "Game")
	virtual void HandleGameClear();

	// Unreal식 추상 함수 : 자식 GameMode들은 이걸 구현해야한다.
	UFUNCTION(BlueprintPure, Category = "Game")
	virtual bool CanClear() const PURE_VIRTUAL(AConcreteAscentGameModeBase::CanClear, return false;);

	// Unreal식 추상 함수 : 자식 GameMode들은 이걸 구현해야한다.
	UFUNCTION(BlueprintPure, Category = "Game")
	virtual bool CanRespawn() const PURE_VIRTUAL(AConcreteAscentGameModeBase::CanRespawn, return false;);

	UFUNCTION(BlueprintPure, Category = "Game")
	virtual FTransform GetRespawnTransform();

	UFUNCTION(BlueprintPure, Category = "Game")
	AConcreteAscentCharacter* GetPlayerCharacter() const { return PlayerCharacter; }

	UFUNCTION(BlueprintPure, Category = "Game")
	ACheckpointActor* GetCurrentCheckpoint() const { return CurrentCheckpoint; }

	UFUNCTION(BlueprintPure, Category = "UI")
	TSubclassOf<UUserWidget> GetInGameHUDClass() const { return InGameHUDClass; }

	UFUNCTION(BlueprintPure, Category = "UI")
	TSubclassOf<UUserWidget> GetGameClearUIClass() const { return GameClearUIClass; }

	UFUNCTION(BlueprintPure, Category = "Game")
	bool IsGameCleared() const { return bGameCleared; }
};
