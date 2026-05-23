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
	const float LandingSpeed = FMath::Abs(GetVelocity().Z);

	Super::Landed(Hit);

	LastLandingVerticalSpeed = LandingSpeed;

	bIsGrounded = true;
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
	// Step 0. Traversal 시작 상태로 전환
	bIsTraversing = true;
	bCanMove = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// Step 0-1. 기존 MovementMode 저장
		PreviousMovementMode = MoveComp->MovementMode;
		PreviousCustomMovementMode = MoveComp->CustomMovementMode;

		// Step 0-2. 기존 이동 속도 제거
		MoveComp->StopMovementImmediately();

		// Step 0-3. Traversal 중에는 Walking의 바닥 보정/중력 영향을 피하기 위해 Flying 사용
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

	// 파쿠르 중 무시했던 장애물 컴포넌트 복구
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
	// Step 6. Traversal Montage가 끝나거나 중단되면 MovementMode 복구
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
	// TODO: 파쿠르 관련 동작 처리
	// 파쿠르 미수행 시 점프 동작
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

void AConcreteAscentCharacter::UpdateTraversalWarpTargets(const FTraversalCheckResult& TraversalResult)
{
	if (!MotionWarpingComponent)
	{
		return;
	}

	// 1. 기본 진행 방향은 FrontLedge -> BackLedge 방향으로 잡음.
	// 이게 가장 안정적인 "넘어가는 방향"이다.
	FVector TraversalForward =
		TraversalResult.BackLedgeLocation - TraversalResult.FrontLedgeLocation;

	TraversalForward.Z = 0.f;

	if (!TraversalForward.Normalize())
	{
		// BackLedge가 없거나 두 위치가 같으면 FrontLedgeNormal 기반으로 fallback
		TraversalForward = -TraversalResult.FrontLedgeNormal.GetSafeNormal();
		TraversalForward.Z = 0.f;
		TraversalForward.Normalize();
	}

	// 2. 혹시 Front/Back이 뒤집혔을 때 현재 캐릭터 전방과 비교해서 보정
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

	// 3. FrontLedge WarpTarget
	if (TraversalResult.bHasFrontLedge)
	{
		FVector FrontTargetLocation =
			TraversalResult.FrontLedgeLocation + FVector(0.f, 0.f, 0.5f);

		/*if (const UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
		{
			FrontTargetLocation.Z -= CapsuleComp->GetScaledCapsuleHalfHeight();
		}*/

		FVector FrontOutward = TraversalResult.FrontLedgeNormal;
		FrontOutward.Z = 0.f;

		if (!FrontOutward.Normalize())
		{
			FrontOutward = -TraversalForward;
		}

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

	// 4. BackLedge는 Hurdle / Vault에서만 사용
	const bool bNeedsBackLedge =
		TraversalResult.ActionType == ETraversalAction::Hurdle ||
		TraversalResult.ActionType == ETraversalAction::Vault;

	FVector BackLedgeTargetLocation =
		TraversalResult.BackLedgeLocation + FVector(0.f, 0.f, 0.5f);

	/*if (const UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		BackLedgeTargetLocation.Z -= CapsuleComp->GetScaledCapsuleHalfHeight();
	}*/

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

	// 5. BackFloor는 Hurdle에서만 사용
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

			// 원본 BP 방식:
			// BackLedge 위치에서 애니메이션상 BackLedge -> BackFloor 추가 이동거리만큼
			// 뒤쪽으로 민 뒤, Z는 실제 바닥 위치를 사용한다.
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
	// Step 5. 몽타주 실행
	// Step 5-0. Montage 유효성 확인
	if (!Montage)
		return 0.f;

	// Step 5-1. Mesh 유효성 확인
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
		return 0.f;

	// Step 5-2. AnimInstance 유효성 확인
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return 0.f;

	// Step 5-3. PlayRate와 StartTime을 반영하여 Montage 재생
	const float MontageLength = AnimInstance->Montage_Play(
		Montage,
		PlayRate,
		EMontagePlayReturnType::MontageLength,
		StartTime
	);

	if (MontageLength <= 0.f)
		return 0.f;

	// Step 5-4. Traversal Montage 종료 시 MovementMode 복구
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AConcreteAscentCharacter::OnTraversalMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

	return MontageLength;
}
