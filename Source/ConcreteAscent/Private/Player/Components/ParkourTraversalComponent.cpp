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

	// 이후 ledge 계산에서 충돌 지점 정보가 필요하므로 마지막 Hit 정보를 함께 저장한다.
	DetectedObstacle = Obstacle;
	LastObstacleHit = Hit;

	return DetectedObstacle;
}

FTraversalChooserInputs UParkourTraversalComponent::EvaluateTraversal(AParkourObstacleBase* ObstacleBase)
{
	// 장애물과 캐릭터 상태를 기반으로 Chooser에 전달할 입력값을 만든다.
	CurrentTraversalResult = BuildTraversalCheckResult(ObstacleBase);
	CurrentTraversalInputs = MakeChooserInputsFromCheckResult(CurrentTraversalResult);

	return CurrentTraversalInputs;
}

bool UParkourTraversalComponent::StartTraversal(FTraversalCheckResult& OutResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Start"));

	if (!OwnerCharacter)
		return false;

	OutResult = FTraversalCheckResult();

	UE_LOG(LogTemp, Warning, TEXT("Obstacle Finding"));

	AParkourObstacleBase* Obstacle = DetectObstacle();
	if (!Obstacle)
		return false;

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

	// Chooser를 통해 현재 상황에 맞는 파쿠르 액션과 후보 몽타주 목록을 얻는다.
	TArray<UAnimMontage*> ValidMontages =
		BuildValidTraversalMontages(CurrentTraversalInputs, CurrentTraversalOutputs);

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

	if (CurrentTraversalOutputs.ActionType == ETraversalAction::LedgeGrab)
	{
		if (!MotionData)
			return false;

		// LedgeGrab은 Chooser 후보가 아니라 MotionData에 지정된 전용 몽타주를 사용한다.
		SelectedMontage = MotionData->GetLedgeGrabMontage();
	}
	else
	{
		if (ValidMontages.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Traversal failed: No valid montages."));
			return false;
		}

		const bool bMotionMatched = MotionMatchTraversal(
			ValidMontages,
			SelectedMontage,
			SelectedStartTime,
			SelectedPlayRate
		);

		if (!bMotionMatched)
		{
			UE_LOG(LogTemp, Warning, TEXT("Traversal failed: Motion match returned null."));
			return false;
		}
	}

	// 실제 재생은 Character가 담당하므로, 여기서는 선택 결과만 TraversalResult에 담아 반환한다.
	const float BranchInStartTime = GetStartTimeFromBranchInEnd(SelectedMontage);

	CurrentTraversalResult.ChosenMontage = SelectedMontage;
	CurrentTraversalResult.StartTime = BranchInStartTime;
	CurrentTraversalResult.PlayRate = 1.f;

	OutResult = CurrentTraversalResult;
	return true;
}

bool UParkourTraversalComponent::StartLedgeGrab(float Direction)
{
	if (!OwnerCharacter)
		return false;

	// TODO: 난간 매달리기 전용 감지, 위치 보정, 몽타주 실행 흐름을 연결해야 한다.
	CurrentTraversalResult.ActionType = ETraversalAction::LedgeGrab;

	return true;
}

void UParkourTraversalComponent::MoveAlongLedge(float Direction)
{
	if (!OwnerCharacter)
		return;

	// TODO: 현재 잡고 있는 난간의 좌우 이동 가능 범위를 확인한 뒤 이동 몽타주를 재생해야 한다.

	return;
}

void UParkourTraversalComponent::ClimbFromLedge()
{
	if (!OwnerCharacter)
		return;

	// TODO: 난간 위쪽 공간을 확인한 뒤 올라가기 몽타주를 재생해야 한다.

	return;
}

void UParkourTraversalComponent::DropFromLedge()
{
	if (!OwnerCharacter)
		return;

	// TODO: 매달린 상태를 해제하고 낙하 상태로 전환해야 한다.

	return;
}

TArray<UAnimMontage*> UParkourTraversalComponent::BuildValidTraversalMontages(
	const FTraversalChooserInputs& Inputs,
	FTraversalChooserOutputs& OutOutputs
)
{
	TArray<UAnimMontage*> Result;

	OutOutputs = FTraversalChooserOutputs();

	// Chooser 평가는 블루프린트에서 수행하고, C++은 결과만 받아서 사용한다.
	BP_EvaluateTraversalChooser(Inputs, OutOutputs, Result);

	return Result;
}

bool UParkourTraversalComponent::MotionMatchTraversal(
	const TArray<UAnimMontage*>& ValidMontages,
	UAnimMontage*& OutMontage,
	float& OutStartTime,
	float& OutPlayRate
) const
{
	OutMontage = nullptr;
	OutStartTime = 0.f;
	OutPlayRate = 1.f;

	if (ValidMontages.IsEmpty())
		return false;

	BP_MotionMatchTraversal(ValidMontages, OutMontage, OutStartTime, OutPlayRate);

	return OutMontage != nullptr;
}

bool UParkourTraversalComponent::FindPoseSearchBranchInEndTime(
	const UAnimMontage* Montage,
	float& OutEndTime
) const
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

		// PoseSearch BranchIn NotifyState를 직접 타입 참조하지 않고 클래스명으로 확인한다.
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

		// 여러 BranchIn 구간이 있을 경우 가장 앞에 있는 구간을 기준으로 사용한다.
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
}

bool UParkourTraversalComponent::CapsuleSweep(
	const FVector& Start,
	const FVector& End,
	ECollisionChannel Channel,
	FHitResult& OutHit
) const
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

bool UParkourTraversalComponent::HasCapsuleRoom(
	const FVector& Start,
	const FVector& End,
	ECollisionChannel Channel,
	FHitResult& OutHit
) const
{
	const bool bHit = CapsuleSweep(Start, End, Channel, OutHit);

	if (!bHit)
		return true;

	return !(OutHit.bBlockingHit || OutHit.bStartPenetrating);
}

FTraversalCheckResult UParkourTraversalComponent::BuildTraversalCheckResult(
	AParkourObstacleBase* ObstacleBase
)
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

	// 장애물 Bounds 기준으로 앞/뒤 Ledge 위치와 Normal 정보를 가져온다.
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

	Result.bHasFrontLedge = true;
	Result.bHasBackLedge = true;

	Result.FrontLedgeLocation = FrontLedgeLocation;
	Result.FrontLedgeNormal = FrontLedgeNormal.GetSafeNormal();

	Result.BackLedgeLocation = BackLedgeLocation;
	Result.BackLedgeNormal = BackLedgeNormal.GetSafeNormal();

	// Chooser에서 사용할 장애물 높이는 캐릭터 캡슐 바닥 기준으로 계산한다.
	const FVector CapsuleLocation = CapsuleComp->GetComponentLocation();
	const float CapsuleBottomZ = CapsuleLocation.Z - CapsuleHalfHeight;
	Result.ObstacleHeight = FMath::Max(0.f, Result.FrontLedgeLocation.Z - CapsuleBottomZ);
	Result.ObstacleDepth = ObstacleDepth;

	Result.HitComponent = LastObstacleHit.GetComponent();

	// FrontLedge 앞쪽에 캐릭터 캡슐이 들어갈 수 있는 공간이 있는지 확인한다.
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

	// BackLedge까지 이동하는 경로에 캐릭터 캡슐이 들어갈 수 있는지 확인한다.
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

	if (!bHasTopRoom)
	{
		Result.bHasBackLedge = false;
		Result.bHasBackFloor = false;

		return Result;
	}

	// 장애물 뒤쪽 아래에 착지 가능한 바닥이 있는지 확인한다.
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

	// BackFloor는 모션워핑의 착지 기준으로 사용할 실제 바닥 표면 위치다.
	Result.BackFloorLocation = FloorHit.ImpactPoint;

	// BackLedgeHeight는 장애물 뒤쪽 ledge와 실제 착지 바닥 사이의 높이 차이다.
	Result.BackLedgeHeight = FMath::Abs(Result.BackLedgeLocation.Z - FloorHit.ImpactPoint.Z);

	return Result;
}

FTraversalChooserInputs UParkourTraversalComponent::MakeChooserInputsFromCheckResult(
	const FTraversalCheckResult& CheckResult
) const
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
