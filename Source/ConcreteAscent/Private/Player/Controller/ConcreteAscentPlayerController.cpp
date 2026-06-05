// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Controller/ConcreteAscentPlayerController.h"
#include "GameMode/ConcreteAscentGameModeBase.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

AConcreteAscentPlayerController::AConcreteAscentPlayerController()
{
	bShowMouseCursor = false;
}

void AConcreteAscentPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CacheInputSubsystem();
	SetupInputMapping();
	SetGameplayInputMode();
	CreateInGameHUD();
}

void AConcreteAscentPlayerController::ShowGameClearUI()
{
	if (GameClearUIWidget)
		return;

	AConcreteAscentGameModeBase* GameMode = Cast<AConcreteAscentGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!GameMode)
		return;

	TSubclassOf<UUserWidget> GameClearUIClass = GameMode->GetGameClearUIClass();
	if (!GameClearUIClass)
		return;

	GameClearUIWidget = CreateWidget<UUserWidget>(this, GameClearUIClass);
	if (!GameClearUIWidget)
		return;

	GameClearUIWidget->AddToViewport(200);
	UGameplayStatics::SetGamePaused(this, true);
	SetUIOnlyInputMode(GameClearUIWidget);
}

void AConcreteAscentPlayerController::CreateInGameHUD()
{
	if (InGameHUDWidget)
		return;

	AConcreteAscentGameModeBase* GameMode =
		Cast<AConcreteAscentGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!GameMode)
		return;

	TSubclassOf<UUserWidget> HUDClass = GameMode->GetInGameHUDClass();
	if (!HUDClass)
		return;

	InGameHUDWidget = CreateWidget<UUserWidget>(this, HUDClass);
	if (!InGameHUDWidget)
		return;

	InGameHUDWidget->AddToViewport(0);
}

void AConcreteAscentPlayerController::CacheInputSubsystem()
{
	InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
}

void AConcreteAscentPlayerController::SetupInputMapping()
{
	if (GameplayMappingContext)
		AddMappingContext(GameplayMappingContext, 0);
}

void AConcreteAscentPlayerController::AddMappingContext(UInputMappingContext* Context, int32 Priority)
{
	if (InputSubsystem && Context)
		InputSubsystem->AddMappingContext(Context, Priority);
}

void AConcreteAscentPlayerController::RemoveMappingContext(UInputMappingContext* Context)
{
	if (InputSubsystem && Context)
		InputSubsystem->RemoveMappingContext(Context);
}

void AConcreteAscentPlayerController::SetGameplayInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AConcreteAscentPlayerController::SetGameAndUIInputMode(UUserWidget* FocusWidget)
{
	FInputModeGameAndUI InputMode;

	if (FocusWidget)
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AConcreteAscentPlayerController::SetUIOnlyInputMode(UUserWidget* FocusWidget)
{
	FInputModeUIOnly InputMode;

	if (FocusWidget)
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AConcreteAscentPlayerController::TogglePauseMenu()
{
	if (bPauseMenuOpen)
		ClosePauseMenu();
	else
		OpenPauseMenu();
}

void AConcreteAscentPlayerController::OpenPauseMenu()
{
	if (bPauseMenuOpen)
		return;

	if (!PauseMenuWidget)
	{
		if (!PauseMenuClass)
			return;

		PauseMenuWidget = CreateWidget<UUserWidget>(this, PauseMenuClass);
	}

	if (!PauseMenuWidget)
		return;

	PauseMenuWidget->AddToViewport(100);
	UGameplayStatics::SetGamePaused(this, true);
	SetGameAndUIInputMode(PauseMenuWidget);

	bPauseMenuOpen = true;
}

void AConcreteAscentPlayerController::ClosePauseMenu()
{
	if (!bPauseMenuOpen)
		return;

	if (PauseMenuWidget)
		PauseMenuWidget->RemoveFromParent();

	UGameplayStatics::SetGamePaused(this, false);

	SetGameplayInputMode();

	bPauseMenuOpen = false;
}