// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ConcreteAscentCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputAction.h"
#include "Player/Components/ParkourTraversalComponent.h"
#include "MotionWarpingComponent.h"

AConcreteAscentCharacter::AConcreteAscentCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	ParkourTraversalComponent = CreateDefaultSubobject<UParkourTraversalComponent>(TEXT("ParkourTraversalComponent"));

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraArm->SetupAttachment(GetRootComponent());
	CameraArm->SocketOffset = FVector(0.0f, 55.0f, 65.0f);
	CameraArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AConcreteAscentCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	bIsGrounded = GetCharacterMovement() ? GetCharacterMovement()->IsMovingOnGround() : true;
}

void AConcreteAscentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGait();

	if (UCharacterMovementComponent* AscentCharacterMovement = GetCharacterMovement())
		bIsGrounded = AscentCharacterMovement->IsMovingOnGround();
}

void AConcreteAscentCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AConcreteAscentCharacter::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AConcreteAscentCharacter::Move);
		}

		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AConcreteAscentCharacter::Look);

		if (JumpInputAction)
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::Jump);

		if (WalkInputAction)
			EnhancedInputComponent->BindAction(WalkInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::ToggleWalk);

		if (SprintInputAction)
		{
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Completed, this, &AConcreteAscentCharacter::StopSprint);
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Canceled, this, &AConcreteAscentCharacter::StopSprint);
		}
	}
}

void AConcreteAscentCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	LastLandingVerticalSpeed = FMath::Abs(GetVelocity().Z);

	bIsGrounded = true;
	bJustLanded = true;

	GetWorldTimerManager().ClearTimer(JustLandedTimerHandle);
	GetWorldTimerManager().SetTimer(JustLandedTimerHandle, this, &AConcreteAscentCharacter::ClearJustLanded, 0.3f, false);
}

void AConcreteAscentCharacter::UpdateGait()
{
	UCharacterMovementComponent* AscentCharacterMovement = GetCharacterMovement();
	if (!AscentCharacterMovement)
	{
		return;
	}

	if (!bCanMove)
	{
		AscentCharacterMovement->MaxWalkSpeed = 0.f;
		return;
	}

	if (bSprint)
	{
		Gait = EGait::Sprint;
		AscentCharacterMovement->MaxWalkSpeed = SprintMaxSpeed;
	}
	else if (bWalk)
	{
		Gait = EGait::Walk;
		AscentCharacterMovement->MaxWalkSpeed = WalkMaxSpeed;
	}
	else
	{
		Gait = EGait::Run;
		AscentCharacterMovement->MaxWalkSpeed = RunMaxSpeed;
	}
}

void AConcreteAscentCharacter::ClearJustLanded()
{
	bJustLanded = false;
}

void AConcreteAscentCharacter::Move(const FInputActionValue& InputValue)
{
	const FVector2D MovementVector = InputValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (MovementVector.Y != 0.0f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);

		AddMovementInput(ForwardDirection, MovementVector.Y);
	}
	if (MovementVector.X != 0.0f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);

		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AConcreteAscentCharacter::Look(const FInputActionValue& InputValue)
{
	const FVector2D LookAxisVector = InputValue.Get<FVector2D>();

	if (LookAxisVector.X != 0.0f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}
	if (LookAxisVector.Y != 0.0f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AConcreteAscentCharacter::ToggleWalk(const FInputActionValue& InputValue)
{
	bWalk = !bWalk;
}

void AConcreteAscentCharacter::StartSprint(const FInputActionValue& InputValue)
{
	bSprint = true;
}

void AConcreteAscentCharacter::StopSprint(const FInputActionValue& InputValue)
{
	bSprint = false;
}

void AConcreteAscentCharacter::Jump()
{
	// TODO: 파쿠르 관련 동작 처리
	// 파쿠르 미수행 시 점프 동작
	Super::Jump();
}

void AConcreteAscentCharacter::RespawnAt(const FTransform& RespawnTransform)
{
	SetActorLocationAndRotation(RespawnTransform.GetLocation(), RespawnTransform.GetRotation().Rotator(), false, nullptr, ETeleportType::TeleportPhysics);

	if (UCharacterMovementComponent* AscentCharacterMovement = GetCharacterMovement())
	{
		AscentCharacterMovement->StopMovementImmediately();
		AscentCharacterMovement->Velocity = FVector::ZeroVector;
	}

	GetWorldTimerManager().ClearTimer(JustLandedTimerHandle);

	bCanMove = true;
	bIsGrounded = true;
	bIsHanging = false;
	bJustLanded = false;
	LastLandingVerticalSpeed = 0.f;
}

void AConcreteAscentCharacter::SetMovementState(EMovementState NewState)
{
	// TODO: 플레이어 상태에 따라 bool 갱신
	return;
}

void AConcreteAscentCharacter::ApplyWarpTarget(const FName WarpTargetName, const FTransform& WarpTargetTransform)
{
	if (MotionWarpingComponent)
	{
		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(WarpTargetName, WarpTargetTransform);
	}
}

float AConcreteAscentCharacter::PlayTraversalMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return 0.f;
	}

	return PlayAnimMontage(Montage);
}