// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ConcreteAscentPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UUserWidget;

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

	UPROPERTY()
	TObjectPtr<UUserWidget> InGameHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameClearUIWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	bool bPauseMenuOpen = false;

	void CreateInGameHUD();

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
	void SetGameAndUIInputMode(UUserWidget* FocusWidget);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetUIOnlyInputMode(UUserWidget* FocusWidget);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameClearUI();
};
