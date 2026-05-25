// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ConcreteAscentCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputAction.h"
#include "Player/Components/ParkourTraversalComponent.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"

namespace
{
	const FName FrontLedgeWarpTargetName(TEXT("FrontLedge"));
	const FName BackLedgeWarpTargetName(TEXT("BackLedge"));
	const FName BackFloorWarpTargetName(TEXT("BackFloor"));

	FTransform MakeTraversalWarpTransformFromForward(const FVector& Location, const FVector& Forward)
	{
		FVector SafeForward = Forward.GetSafeNormal();

		if (SafeForward.IsNearlyZero())
			SafeForward = FVector::ForwardVector;

		FRotator Rotation = SafeForward.Rotation();
		Rotation.Pitch = 0.f;
		Rotation.Roll = 0.f;

		return FTransform(Rotation, Location, FVector::OneVector);
	}
}

AConcreteAscentCharacter::AConcreteAscentCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

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

	ParkourTraversalComponent = FindComponentByClass<UParkourTraversalComponent>();
	if (!ParkourTraversalComponent)
		UE_LOG(LogTemp, Warning, TEXT("ParkourTraversalComponent is not attached to this character."));
}

void AConcreteAscentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGait();
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
	const float LandingSpeed = FMath::Abs(GetVelocity().Z);

	Super::Landed(Hit);

	LastLandingVerticalSpeed = LandingSpeed;
	bJustLanded = true;

	GetWorldTimerManager().ClearTimer(JustLandedTimerHandle);
	GetWorldTimerManager().SetTimer(
		JustLandedTimerHandle,
		this,
		&AConcreteAscentCharacter::ClearJustLanded,
		0.3f,
		false
	);
}

void AConcreteAscentCharacter::UpdateGait()
{
	if (bIsTraversing)
		return;

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

void AConcreteAscentCharacter::BeginTraversal()
{
	// 파쿠르 동작 중에는 일반 이동 입력과 CharacterMovement의 기본 이동 처리를 잠시 막는다.
	bIsTraversing = true;
	bCanMove = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// 기존 이동 관성을 제거해 파쿠르 몽타주와 모션워핑이 안정적으로 적용되도록 한다.
		MoveComp->StopMovementImmediately();

		// 파쿠르 중에는 Walking의 바닥 보정과 중력 영향을 피하기 위해 Flying 모드를 사용한다.
		MoveComp->SetMovementMode(MOVE_Flying);
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->IgnoreComponentWhenMoving(
			CurrentTraversalResult.HitComponent.Get(),
			true
		);
	}
}

void AConcreteAscentCharacter::EndTraversal()
{
	bIsTraversing = false;
	bCanMove = true;

	// 파쿠르 중 임시로 무시했던 장애물 충돌을 복구한다.
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		if (CurrentTraversalResult.HitComponent)
		{
			CapsuleComp->IgnoreComponentWhenMoving(
				CurrentTraversalResult.HitComponent.Get(),
				false
			);
		}
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		switch (CurrentTraversalResult.ActionType)
		{
		case ETraversalAction::Vault:
			MoveComp->SetMovementMode(MOVE_Falling);
			break;

		case ETraversalAction::Hurdle:
		case ETraversalAction::Mantle:
		default:
			MoveComp->SetMovementMode(MOVE_Walking);
			break;
		}
	}

	UpdateGait();
}

void AConcreteAscentCharacter::OnTraversalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 몽타주가 끝나거나 중단되면 파쿠르 상태를 정리한다.
	EndTraversal();
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
	UE_LOG(LogTemp, Warning, TEXT("Jump"));

	if (ParkourTraversalComponent)
	{
		FTraversalCheckResult TraversalResult;
		if (ParkourTraversalComponent->StartTraversal(TraversalResult))
		{
			CurrentTraversalResult = TraversalResult;

			BeginTraversal();

			UpdateTraversalWarpTargets(CurrentTraversalResult);

			const float MontageLength = PlayTraversalMontage(
				CurrentTraversalResult.ChosenMontage,
				CurrentTraversalResult.PlayRate,
				CurrentTraversalResult.StartTime
			);

			if (MontageLength > 0.f)
				return;

			EndTraversal();
		}
	}

	// 파쿠르 동작을 시작하지 못한 경우 일반 점프를 수행한다.
	Super::Jump();
}

void AConcreteAscentCharacter::RespawnAt(const FTransform& RespawnTransform)
{
	SetActorLocationAndRotation(
		RespawnTransform.GetLocation(),
		RespawnTransform.GetRotation().Rotator(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	if (UCharacterMovementComponent* AscentCharacterMovement = GetCharacterMovement())
	{
		AscentCharacterMovement->StopMovementImmediately();
		AscentCharacterMovement->Velocity = FVector::ZeroVector;
	}

	GetWorldTimerManager().ClearTimer(JustLandedTimerHandle);

	bCanMove = true;
	bIsHanging = false;
	bJustLanded = false;
	LastLandingVerticalSpeed = 0.f;
}

void AConcreteAscentCharacter::UpdateTraversalWarpTargets(const FTraversalCheckResult& TraversalResult)
{
	if (!MotionWarpingComponent)
	{
		return;
	}

	// 기본 진행 방향은 FrontLedge에서 BackLedge로 향하는 방향을 사용한다.
	FVector TraversalForward =
		TraversalResult.BackLedgeLocation - TraversalResult.FrontLedgeLocation;

	TraversalForward.Z = 0.f;

	if (!TraversalForward.Normalize())
	{
		// BackLedge 정보가 없거나 두 위치가 거의 같으면 FrontLedgeNormal을 기준으로 보정한다.
		TraversalForward = -TraversalResult.FrontLedgeNormal.GetSafeNormal();
		TraversalForward.Z = 0.f;
		TraversalForward.Normalize();
	}

	// 계산된 진행 방향이 현재 캐릭터 전방과 반대라면 뒤집어서 보정한다.
	FVector ActorForward = GetActorForwardVector();
	ActorForward.Z = 0.f;
	ActorForward.Normalize();

	if (!ActorForward.IsNearlyZero() &&
		FVector::DotProduct(TraversalForward, ActorForward) < 0.f)
	{
		TraversalForward *= -1.f;
	}

	if (TraversalForward.IsNearlyZero())
	{
		TraversalForward = ActorForward;
	}

	if (TraversalForward.IsNearlyZero())
	{
		TraversalForward = FVector::ForwardVector;
	}

	if (TraversalResult.bHasFrontLedge)
	{
		FVector FrontTargetLocation =
			TraversalResult.FrontLedgeLocation + FVector(0.f, 0.f, 0.5f);

		FVector FrontOutward = TraversalResult.FrontLedgeNormal;
		FrontOutward.Z = 0.f;

		if (!FrontOutward.Normalize())
		{
			FrontOutward = -TraversalForward;
		}

		// FrontLedge 타겟이 장애물 안쪽으로 들어가지 않도록 바깥 방향으로 약간 밀어낸다.
		FrontTargetLocation += FrontOutward * FrontLedgeOutwardOffset;

		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(
			FrontLedgeWarpTargetName,
			MakeTraversalWarpTransformFromForward(
				FrontTargetLocation,
				TraversalForward
			)
		);
	}
	else
	{
		MotionWarpingComponent->RemoveWarpTarget(FrontLedgeWarpTargetName);
	}

	// BackLedge 타겟은 장애물을 넘어가는 동작에서만 사용한다.
	const bool bNeedsBackLedge =
		TraversalResult.ActionType == ETraversalAction::Hurdle ||
		TraversalResult.ActionType == ETraversalAction::Vault;

	FVector BackLedgeTargetLocation =
		TraversalResult.BackLedgeLocation + FVector(0.f, 0.f, 0.5f);

	if (bNeedsBackLedge && TraversalResult.bHasBackLedge)
	{
		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(
			BackLedgeWarpTargetName,
			MakeTraversalWarpTransformFromForward(
				BackLedgeTargetLocation,
				TraversalForward
			)
		);
	}
	else
	{
		MotionWarpingComponent->RemoveWarpTarget(BackLedgeWarpTargetName);
	}

	// BackFloor 타겟은 Hurdle처럼 장애물 뒤쪽 바닥까지 이어지는 동작에서 사용한다.
	const bool bNeedsBackFloor =
		TraversalResult.ActionType == ETraversalAction::Hurdle;

	if (bNeedsBackFloor && TraversalResult.bHasBackFloor)
	{
		FVector BackFloorTargetLocation = TraversalResult.BackFloorLocation;

		float AnimatedDistanceToBackLedge = 0.f;
		float AnimatedDistanceToBackFloor = 0.f;

		const bool bGotBackLedgeDistance =
			BP_GetDistanceFromLedgeAtWarpEnd(
				TraversalResult.ChosenMontage,
				BackLedgeWarpTargetName,
				AnimatedDistanceToBackLedge
			);

		const bool bGotBackFloorDistance =
			BP_GetDistanceFromLedgeAtWarpEnd(
				TraversalResult.ChosenMontage,
				BackFloorWarpTargetName,
				AnimatedDistanceToBackFloor
			);

		if (bGotBackLedgeDistance && bGotBackFloorDistance)
		{
			const float ExtraFloorDistance = FMath::Abs(
				AnimatedDistanceToBackFloor - AnimatedDistanceToBackLedge
			);

			FVector BackFloorDirection = TraversalResult.BackLedgeNormal;
			BackFloorDirection.Z = 0.f;

			if (!BackFloorDirection.Normalize())
			{
				BackFloorDirection = TraversalForward;
			}

			// 애니메이션상의 BackLedge-BackFloor 거리 차이를 실제 바닥 타겟 위치에 반영한다.
			const FVector BackFloorXYLocation =
				TraversalResult.BackLedgeLocation
				+ BackFloorDirection * ExtraFloorDistance;

			BackFloorTargetLocation.X = BackFloorXYLocation.X;
			BackFloorTargetLocation.Y = BackFloorXYLocation.Y;
			BackFloorTargetLocation.Z = TraversalResult.BackFloorLocation.Z;
		}

		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(
			BackFloorWarpTargetName,
			MakeTraversalWarpTransformFromForward(
				BackFloorTargetLocation,
				TraversalForward
			)
		);

		UE_LOG(LogTemp, Warning,
			TEXT("BackFloor Warp | BackLedgeDist:%.2f BackFloorDist:%.2f GotLedge:%d GotFloor:%d RawFloor:%s TargetFloor:%s"),
			AnimatedDistanceToBackLedge,
			AnimatedDistanceToBackFloor,
			bGotBackLedgeDistance,
			bGotBackFloorDistance,
			*TraversalResult.BackFloorLocation.ToString(),
			*BackFloorTargetLocation.ToString()
		);
	}
	else
	{
		MotionWarpingComponent->RemoveWarpTarget(BackFloorWarpTargetName);
	}
}

float AConcreteAscentCharacter::PlayTraversalMontage(UAnimMontage* Montage, float PlayRate, float StartTime)
{
	if (!Montage)
		return 0.f;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
		return 0.f;

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return 0.f;

	const float MontageLength = AnimInstance->Montage_Play(
		Montage,
		PlayRate,
		EMontagePlayReturnType::MontageLength,
		StartTime
	);

	if (MontageLength <= 0.f)
		return 0.f;

	// 파쿠르 몽타주가 끝나면 캐릭터 상태와 MovementMode를 복구한다.
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AConcreteAscentCharacter::OnTraversalMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

	return MontageLength;
}