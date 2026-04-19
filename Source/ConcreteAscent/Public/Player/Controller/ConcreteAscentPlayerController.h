// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ConcreteAscentPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API AConcreteAscentPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AConcreteAscentPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> GameplayMappingContext;

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	void CacheInputSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetupInputMapping();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void AddMappingContext(UInputMappingContext* Context, int32 Priority = 0);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void RemoveMappingContext(UInputMappingContext* Context);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetGameplayInputMode();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetUIInputMode();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePauseMenu();
};
