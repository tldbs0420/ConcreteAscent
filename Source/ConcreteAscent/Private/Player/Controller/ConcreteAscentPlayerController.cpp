// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Controller/ConcreteAscentPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

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

void AConcreteAscentPlayerController::SetUIInputMode()
{
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AConcreteAscentPlayerController::OpenPauseMenu()
{
	UGameplayStatics::SetGamePaused(this, true);
	SetUIInputMode();

	// TODO: 일시 정지 메뉴 보여주기
}

void AConcreteAscentPlayerController::ClosePauseMenu()
{
	UGameplayStatics::SetGamePaused(this, false);
	SetGameplayInputMode();

	// TODO: 일시 정지 메뉴 없애기
}