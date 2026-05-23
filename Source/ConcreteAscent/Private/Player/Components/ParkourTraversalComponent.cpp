// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Components/ParkourTraversalComponent.h"
#include "Player/ConcreteAscentCharacter.h"
#include "Objects/Obstacle/LedgeObstacle.h"
#include "Objects/Obstacle/ParkourObstacleBase.h"
#include "Data/ParkourMotionData.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimTypes.h"
#include "PoseSearch/PoseSearchAnimNotifies.h"

UParkourTraversalComponent::UParkourTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParkourTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AConcreteAscentCharacter>(GetOwner());
}

AParkourObstacleBase* UParkourTraversalComponent::DetectObstacle()
{
	// TODO: 파쿠르 대상 탐색
	// 1. 플레이어 전면부 탐지
	// 2. AParkourObstacleBase 있는지 확인
	// 3.AParkourObstacleBase 캐싱 및 리턴
	if (!OwnerCharacter || !GetWorld())
		return nullptr;

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();

	const FVector Start = ActorLocation + FVector(0.f, 0.f, TraceOriginZOffset);
	const FVector End = Start + Forward * TraceForwardDistance;

	FHitResult Hit;
	const bool bHit = CapsuleSweep(Start, End, TraversableTraceChannel, Hit);

	if (!bHit || !Hit.bBlockingHit || !Hit.GetActor())
	{
		DetectedObstacle = nullptr;
		LastObstacleHit = FHitResult();
		return nullptr;
	}

	AParkourObstacleBase* Obstacle = Cast<AParkourObstacleBase>(Hit.GetActor());
	if (!Obstacle)
	{
		DetectedObstacle = nullptr;
		LastObstacleHit = FHitResult();
		return nullptr;
	}

	DetectedObstacle = Obstacle;
	LastObstacleHit = Hit;

	return DetectedObstacle;
}

FTraversalChooserInputs UParkourTraversalComponent::EvaluateTraversal(AParkourObstacleBase* ObstacleBase)
{
	// TODO: Chooser용 Input struct 생성 후 내용 채워넣고 판단

	CurrentTraversalResult = BuildTraversalCheckResult(ObstacleBase);
	CurrentTraversalInputs = MakeChooserInputsFromCheckResult(CurrentTraversalResult);

	return CurrentTraversalInputs;
}

bool UParkourTraversalComponent::StartTraversal(FTraversalCheckResult& OutResult)
{

	// TODO: 파쿠르 처리
	// Step 1. 전면에 물체 확인
	// Step 2. 물체에 해당하는 파쿠르 판별
	// Step 3-1. Ledge면 Ledge 동작으로 넘어감
	// Step 3-2. 아니면 MotionMatch로 Montage 판별
	// Step 4. 모션워핑 좌표 수정
	// Step 5. 몽타주 실행
	//
	// 현재 함수에서는 Step 1 ~ Step 3까지만 처리한다.
	// Step 4. 모션워핑 좌표 수정
	// Step 5. 몽타주 실행
	// 은 Player 쪽 Jump 입력 흐름에서 처리한다.

	UE_LOG(LogTemp, Warning, TEXT("Start"));

	if (!OwnerCharacter)
		return false;

	OutResult = FTraversalCheckResult();

	// Step 1. 전면에 물체 확인
	UE_LOG(LogTemp, Warning, TEXT("Obstacle Finding"));

	AParkourObstacleBase* Obstacle = DetectObstacle();
	if (!Obstacle)
		return false;

	// Step 2. 물체에 해당하는 파쿠르 판별용 데이터 생성
	UE_LOG(LogTemp, Warning, TEXT("Traversal Evaluate"));

	CurrentTraversalInputs = EvaluateTraversal(Obstacle);

	if (!CurrentTraversalInputs.bHasFrontLedge)
		return false;

		UE_LOG(LogTemp, Warning,
		TEXT("ChooserInput | Action:%d MoveMode:%d Front:%d Back:%d Floor:%d Height:%.2f Depth:%.2f BackHeight:%.2f Speed:%.2f"),
		static_cast<uint8>(CurrentTraversalInputs.ActionType),
		static_cast<uint8>(CurrentTraversalInputs.MovementMode.GetValue()),
		CurrentTraversalInputs.bHasFrontLedge,
		CurrentTraversalInputs.bHasBackLedge,
		CurrentTraversalInputs.bHasBackFloor,
		CurrentTraversalInputs.ObstacleHeight,
		CurrentTraversalInputs.ObstacleDepth,
		CurrentTraversalInputs.BackLedgeHeight,
		CurrentTraversalInputs.Speed
	);

	// Step 3. Chooser를 통해 가능한 Traversal Montage 목록과 ActionType 결정
	TArray<UAnimMontage*> ValidMontages =
		BuildValidTraversalMontages(CurrentTraversalInputs, CurrentTraversalOutputs);

	// Step 3-0. Chooser가 CurrentTraversalInputs.ActionType을 갱신했다고 가정.
	CurrentTraversalInputs.ActionType = CurrentTraversalOutputs.ActionType;
	CurrentTraversalResult.ActionType = CurrentTraversalOutputs.ActionType;

	UE_LOG(LogTemp, Warning,
		TEXT("Traversal | Action:%d MoveMode:%d Front:%d Back:%d Floor:%d Height:%.2f Depth:%.2f BackHeight:%.2f Speed:%.2f Montages:%d"),
		static_cast<uint8>(CurrentTraversalOutputs.ActionType),
		static_cast<uint8>(CurrentTraversalInputs.MovementMode.GetValue()),
		CurrentTraversalInputs.bHasFrontLedge,
		CurrentTraversalInputs.bHasBackLedge,
		CurrentTraversalInputs.bHasBackFloor,
		CurrentTraversalInputs.ObstacleHeight,
		CurrentTraversalInputs.ObstacleDepth,
		CurrentTraversalInputs.BackLedgeHeight,
		CurrentTraversalInputs.Speed,
		ValidMontages.Num()
	);

	if (CurrentTraversalOutputs.ActionType == ETraversalAction::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Traversal failed: ActionType is None."));
		return false;
	}

	if (OwnerCharacter)
		BP_UpdatePoseSearchPlayerActor(OwnerCharacter);

	UAnimMontage* SelectedMontage = nullptr;
	float SelectedStartTime = 0.f;
	float SelectedPlayRate = 1.f;

	// Step 3-1. LedgeGrab이면 Ledge 전용 Montage 선택
	if (CurrentTraversalOutputs.ActionType == ETraversalAction::LedgeGrab)
	{
		if (!MotionData)
			return false;

		SelectedMontage = MotionData->GetLedgeGrabMontage();
	}

	// Step 3-2. 일반 Traversal이면 MotionMatch로 Montage 선택
	else
	{
		if (ValidMontages.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Traversal failed: No valid montages."));
			return false;
		}

		const bool bMotionMatched = MotionMatchTraversal(ValidMontages, SelectedMontage, SelectedStartTime, SelectedPlayRate);

		if (!bMotionMatched)
		{
			UE_LOG(LogTemp, Warning, TEXT("Traversal failed: Motion match returned null."));
			return false;
		}
	}

	// Step 3-3. Player가 실행할 수 있도록 Result에 Montage 정보 저장
	const float BranchInStartTime =
		GetStartTimeFromBranchInEnd(SelectedMontage);

	CurrentTraversalResult.ChosenMontage = SelectedMontage;
	CurrentTraversalResult.StartTime = BranchInStartTime;
	CurrentTraversalResult.PlayRate = 1.f;

	// Step 3-4. 최종 Traversal 결과 반환
	OutResult = CurrentTraversalResult;
	return true;
}

bool UParkourTraversalComponent::StartLedgeGrab(float Direction)
{
	// TODO: 매달리기 몽타주 실행
	// MotionWarping으로 위치 조정
	if (!OwnerCharacter)
		return false;

	CurrentTraversalResult.ActionType = ETraversalAction::LedgeGrab;

	const FTransform WarpTarget =
		BuildWarpTargetFromCheckResult(CurrentTraversalResult);

	ApplyTraversalWarpTarget(WarpTarget);

	PlayLedgeMontage(ETraversalAction::LedgeGrab);

	return true;
}

void UParkourTraversalComponent::MoveAlongLedge(float Direction)
{
	if (!OwnerCharacter)
		return;

	// TODO: ALedgeObstacle로 가로 폭 확인하여 이동 가능한지 확인 후 몽타주 처리

	return;
}

void UParkourTraversalComponent::ClimbFromLedge()
{
	if (!OwnerCharacter)
		return;

	// TODO: 위의 공간이 있는지 확인 후 기어 올라가기 수행

	return;
}

void UParkourTraversalComponent::DropFromLedge()
{
	if (!OwnerCharacter)
		return;

	// TODO: 난간에서 떨어지기 수행

	return;
}

TArray<UAnimMontage*> UParkourTraversalComponent::BuildValidTraversalMontages(const FTraversalChooserInputs& Inputs, FTraversalChooserOutputs& OutOutputs)
{
	TArray<UAnimMontage*> Result;

	OutOutputs = FTraversalChooserOutputs();

	// TODO: 파쿠르 모션을 Chooser를 통해 배열로 리턴
	BP_EvaluateTraversalChooser(Inputs, OutOutputs, Result);

	return Result;
}

//FTransform UParkourTraversalComponent::BuildWarpTarget(const FTraversalChooserInputs& Inputs)
//{
//	if (!DetectedObstacle)
//		return FTransform::Identity;
//
//	// MotionWarping 할 곳을 계산하여 반영
//	return FTransform::Identity;
//}

bool UParkourTraversalComponent::MotionMatchTraversal(const TArray<UAnimMontage*>& ValidMontages, UAnimMontage*& OutMontage, float& OutStartTime, float& OutPlayRate) const
{
	OutMontage = nullptr;
	OutStartTime = 0.f;
	OutPlayRate = 1.f;

	if (ValidMontages.IsEmpty())
		return false;

	BP_MotionMatchTraversal(ValidMontages, OutMontage, OutStartTime, OutPlayRate);

	return OutMontage != nullptr;
}

bool UParkourTraversalComponent::FindPoseSearchBranchInEndTime(const UAnimMontage* Montage, float& OutEndTime) const
{
	OutEndTime = 0.f;

	if (!Montage)
	{ 
		return false;
	}

	bool bFound = false;
	float EarliestStartTime = TNumericLimits<float>::Max();

	for (const FAnimNotifyEvent& NotifyEvent : Montage->Notifies)
	{
		const UAnimNotifyState* NotifyState = NotifyEvent.NotifyStateClass;
		if (!NotifyState)
		{
			continue;
		}

		// PoseSearch 모듈 include 문제를 피하려고 이름 기반으로 검사.
		// 클래스명은 보통 AnimNotifyState_PoseSearchBranchIn 계열이다.
		const FString NotifyClassName = NotifyState->GetClass()->GetName();
		if (!NotifyClassName.Contains(TEXT("PoseSearchBranchIn")))
		{
			continue;
		}

		const float StartTime = NotifyEvent.GetTriggerTime();
		const float EndTime = NotifyEvent.GetEndTriggerTime();

		if (EndTime <= StartTime)
		{
			continue;
		}

		// 여러 개 있으면 가장 앞의 BranchIn을 사용
		if (StartTime < EarliestStartTime)
		{
			EarliestStartTime = StartTime;
			OutEndTime = EndTime;
			bFound = true;
		}
	}

	return bFound;
}

float UParkourTraversalComponent::GetStartTimeFromBranchInEnd(const UAnimMontage* Montage) const
{
	float BranchInEndTime = 0.f;

	if (!FindPoseSearchBranchInEndTime(Montage, BranchInEndTime))
	{
		return 0.f;
	}

	return FMath::Max(
		0.f,
		BranchInEndTime - BranchInEndStartOffset
	);
}

void UParkourTraversalComponent::PlaySelectedTraversalMontage(UAnimMontage* Montage)
{
	if (OwnerCharacter && Montage)
		OwnerCharacter->PlayTraversalMontage(Montage);
}

void UParkourTraversalComponent::PlayLedgeMontage(ETraversalAction Action)
{
	if (!MotionData)
		return;

	UAnimMontage* Montage = nullptr;
	switch (Action)
	{
	case ETraversalAction::LedgeGrab:
		Montage = MotionData->GetLedgeGrabMontage();
		break;
	case ETraversalAction::LedgeClimbUp:
		Montage = MotionData->GetLedgeClimbUpMontage();
		break;
	case ETraversalAction::LedgeMoveLeft:
		Montage = MotionData->GetLedgeMoveMontage(-1);
		break;
	case ETraversalAction::LedgeMoveRight:
		Montage = MotionData->GetLedgeMoveMontage(1);
		break;
	case ETraversalAction::LedgeDrop:
		Montage = MotionData->GetLedgeDropMontage();
		break;
	default:
		break;
	}

	// TODO: MotionWarping 수행

	PlaySelectedTraversalMontage(Montage);
}

bool UParkourTraversalComponent::CapsuleSweep(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const
{
	if (!OwnerCharacter || !GetWorld())
		return false;

	const UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
		return false;

	const float Radius = CapsuleComp->GetScaledCapsuleRadius();
	const float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	const FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ParkourCapsuleSweep), false);
	Params.AddIgnoredActor(OwnerCharacter);

	return GetWorld()->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		FQuat::Identity,
		Channel,
		Shape,
		Params
	);
}

bool UParkourTraversalComponent::HasCapsuleRoom(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const
{
	const bool bHit = CapsuleSweep(Start, End, Channel, OutHit);

	if (!bHit)
		return true;

	return !(OutHit.bBlockingHit || OutHit.bStartPenetrating);
}

FTraversalCheckResult UParkourTraversalComponent::BuildTraversalCheckResult(AParkourObstacleBase* ObstacleBase)
{
	FTraversalCheckResult Result;

	if (!OwnerCharacter || !ObstacleBase)
		return Result;

	const UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
		return Result;

	UE_LOG(LogTemp, Warning, TEXT("TraversalCheckStart"));

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	FVector FrontLedgeLocation;
	FVector FrontLedgeNormal;
	FVector BackLedgeLocation;
	FVector BackLedgeNormal;
	float ObstacleHeight = 0.f;
	float ObstacleDepth = 0.f;

	// Step 3.1
	// Obstacle에게 자신의 TraversalBounds 기준 Ledge 위치, Normal, 높이, 깊이 정보를 요청한다.
	const bool bGotLedgeData = ObstacleBase->GetTraversalLedgeData(
		LastObstacleHit,
		ActorLocation,
		FrontLedgeLocation,
		FrontLedgeNormal,
		BackLedgeLocation,
		BackLedgeNormal,
		ObstacleHeight,
		ObstacleDepth
	);

	if (!bGotLedgeData)
		return Result;

	// Step 3.1-1
	// Obstacle이 계산한 값을 Result에 그대로 적용한다.
	Result.bHasFrontLedge = true;
	Result.bHasBackLedge = true;

	Result.FrontLedgeLocation = FrontLedgeLocation;
	Result.FrontLedgeNormal = FrontLedgeNormal.GetSafeNormal();

	Result.BackLedgeLocation = BackLedgeLocation;
	Result.BackLedgeNormal = BackLedgeNormal.GetSafeNormal();

	const FVector CapsuleLocation = CapsuleComp->GetComponentLocation();
	const float CapsuleBottomZ = CapsuleLocation.Z - CapsuleHalfHeight;
	Result.ObstacleHeight = FMath::Max(0.f, Result.FrontLedgeLocation.Z - CapsuleBottomZ);
	Result.ObstacleDepth = ObstacleDepth;

	Result.HitComponent = LastObstacleHit.GetComponent();

	// Step 3.2
	// Front Ledge 앞쪽에 캐릭터 캡슐이 들어갈 공간이 있는지 확인.
	const FVector FrontRoomLocation =
		Result.FrontLedgeLocation
		+ Result.FrontLedgeNormal * (CapsuleRadius + LedgeRoomPadding)
		+ FVector(0.f, 0.f, CapsuleHalfHeight + LedgeRoomPadding);

	FHitResult FrontRoomHit;
	const bool bHasFrontRoom = HasCapsuleRoom(
		ActorLocation,
		FrontRoomLocation,
		RoomTraceChannel,
		FrontRoomHit
	);

	UE_LOG(LogTemp, Warning, TEXT("bHasFrontRoom"));

	if (!bHasFrontRoom)
	{
		Result.bHasFrontLedge = false;
		return Result;
	}

	// Step 3.3
	// ObstacleHeight는 Obstacle이 이미 계산해서 넘겨준 값을 사용한다.
	// 따라서 ActorTopLocation 기준으로 다시 계산하지 않는다.

	// Step 3.4
	// Back Ledge 뒤쪽에 캐릭터 캡슐이 들어갈 공간이 있는지 확인.
	const FVector BackRoomLocation =
		Result.BackLedgeLocation
		+ Result.BackLedgeNormal * (CapsuleRadius + LedgeRoomPadding)
		+ FVector(0.f, 0.f, CapsuleHalfHeight + LedgeRoomPadding);

	FHitResult TopSweepHit;
	const bool bHasTopRoom = HasCapsuleRoom(
		FrontRoomLocation,
		BackRoomLocation,
		RoomTraceChannel,
		TopSweepHit
	);

	// Step 3.5
	// Back Ledge까지 갈 수 없는 경우, BackLedge와 BackFloor는 사용할 수 없는 것으로 처리한다.
	// ObstacleDepth 자체는 Obstacle이 제공한 실제 깊이 값을 유지한다.
	if (!bHasTopRoom)
	{
		Result.bHasBackLedge = false;
		Result.bHasBackFloor = false;

		return Result;
	}

	// Step 3.6
	// Back Ledge 뒤쪽 아래에 바닥이 있는지 확인.
	// BackFloor는 장애물 자체 정보가 아니라 월드 충돌과 캐릭터 캡슐 기준으로 판단한다.
	const FVector FloorTraceStart =
		BackRoomLocation + FVector(0.f, 0.f, FloorCheckExtraDistance);

	const float FloorCheckDistance =
		CapsuleHalfHeight
		+ Result.ObstacleHeight * 0.5f
		+ FloorCheckExtraDistance;

	const FVector FloorTraceEnd =
		FloorTraceStart - FVector(0.f, 0.f, FloorCheckDistance);

	FHitResult FloorHit;
	const bool bFloorHit = CapsuleSweep(
		FloorTraceStart,
		FloorTraceEnd,
		RoomTraceChannel,
		FloorHit
	);

	if (!bFloorHit || !FloorHit.bBlockingHit)
	{
		Result.bHasBackFloor = false;
		return Result;
	}

	Result.bHasBackFloor = true;

	// BackFloor는 착지할 바닥 표면 위치로 저장한다.
// FloorHit.Location은 캡슐 중심에 가까우므로, Motion Warping 기준이 높게 잡힐 수 있다.
	Result.BackFloorLocation = FloorHit.ImpactPoint;

	// 높이 계산용.
	// BackLedgeHeight는 실제 바닥 표면 기준으로 계산한다.
	Result.BackLedgeHeight = FMath::Abs(Result.BackLedgeLocation.Z - FloorHit.ImpactPoint.Z);

	return Result;
}

FTraversalChooserInputs UParkourTraversalComponent::MakeChooserInputsFromCheckResult(const FTraversalCheckResult& CheckResult) const
{
	FTraversalChooserInputs Inputs;

	Inputs.ActionType = ETraversalAction::None;

	Inputs.bHasFrontLedge = CheckResult.bHasFrontLedge;
	Inputs.bHasBackLedge = CheckResult.bHasBackLedge;
	Inputs.bHasBackFloor = CheckResult.bHasBackFloor;

	Inputs.ObstacleHeight = CheckResult.ObstacleHeight;
	Inputs.ObstacleDepth = CheckResult.ObstacleDepth;
	Inputs.BackLedgeHeight = CheckResult.BackLedgeHeight;

	if (OwnerCharacter)
	{
		Inputs.Speed = OwnerCharacter->GetVelocity().Size2D();

		if (const UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
			Inputs.MovementMode = MoveComp->MovementMode;
		Inputs.Gait = OwnerCharacter->GetGait();
	}

	return Inputs;
}

FTransform UParkourTraversalComponent::BuildWarpTargetFromCheckResult(const FTraversalCheckResult& CheckResult) const
{
	if (!OwnerCharacter)
		return FTransform::Identity;

	FVector TargetLocation = CheckResult.FrontLedgeLocation;
	FVector TargetNormal = CheckResult.FrontLedgeNormal;

	switch (CheckResult.ActionType)
	{
	case ETraversalAction::Vault:
	case ETraversalAction::Hurdle:
		// 넘어가는 계열은 보통 Back Ledge 또는 Back Floor 쪽을 목표로 둠.
		if (CheckResult.bHasBackFloor)
		{
			TargetLocation = CheckResult.BackFloorLocation;
			TargetNormal = CheckResult.BackLedgeNormal;
		}
		else if (CheckResult.bHasBackLedge)
		{
			TargetLocation = CheckResult.BackLedgeLocation;
			TargetNormal = CheckResult.BackLedgeNormal;
		}
		break;

	case ETraversalAction::Mantle:
	case ETraversalAction::LedgeGrab:
		// 매달리기/기어오르기 계열은 Front Ledge 기준.
		TargetLocation = CheckResult.FrontLedgeLocation;
		TargetNormal = CheckResult.FrontLedgeNormal;
		break;

	default:
		break;
	}

	const FRotator TargetRotation = (-TargetNormal).Rotation();

	return FTransform(TargetRotation, TargetLocation, FVector::OneVector);
}

void UParkourTraversalComponent::ApplyTraversalWarpTarget(const FTransform& WarpTarget)
{
	if (!OwnerCharacter)
		return;

	UMotionWarpingComponent* MotionWarpingComp =
		OwnerCharacter->FindComponentByClass<UMotionWarpingComponent>();

	if (!MotionWarpingComp)
		return;

	MotionWarpingComp->AddOrUpdateWarpTargetFromTransform(
		TraversalWarpTargetName,
		WarpTarget
	);
}
