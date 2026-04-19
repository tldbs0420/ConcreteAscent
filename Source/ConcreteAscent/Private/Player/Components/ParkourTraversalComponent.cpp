// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Components/ParkourTraversalComponent.h"
#include "Player/ConcreteAscentCharacter.h"
#include "Objects/Obstacle/LedgeObstacle.h"
#include "Objects/Obstacle/ParkourObstacleBase.h"
#include "Data/ParkourMotionData.h"

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
	return DetectedObstacle;
}

FTraversalChooserInputs UParkourTraversalComponent::EvaluateTraversal(AParkourObstacleBase* ObstacleBase)
{
	// TODO: Chooser용 Input struct 생성 후 내용 채워넣고 판단

	FTraversalChooserInputs Inputs;
	return Inputs;
}

bool UParkourTraversalComponent::StartTraversal()
{
	// TODO: 파쿠르 처리
	// 1. 전면에 물체 확인
	// 2. 물체에 해당하는 파쿠르 판별
	// 3-1. ledge면 ledge 동작으로 넘어감
	// 3-2. 아니면 MotionMatch로 Montage 판별
	// 4. 모션워핑 좌표 수정
	// 5. 몽타주 실행

	return false;
}

bool UParkourTraversalComponent::StartLedgeGrab(float Direction)
{
	// TODO: 매달리기 몽타주 실행
	// MotionWarping으로 위치 조정
	return true;
}

void UParkourTraversalComponent::MoveAlongLedge(float Direction)
{
	if (!OwnerCharacter || OwnerCharacter->GetMovementState() != EMovementState::Hanging)
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

FTransform UParkourTraversalComponent::BuildWarpTarget(const FTraversalChooserInputs& Inputs) const
{
	if (!DetectedObstacle)
		return FTransform::Identity;

	// MotionWarping 할 곳을 계산하여 반영
	return FTransform::Identity;
}

TArray<UAnimMontage*> UParkourTraversalComponent::BuildValidTraversalMontages(const FTraversalChooserInputs& Inputs) const
{
	TArray<UAnimMontage*> Result;
	if (!MotionData)
		return Result;

	// TODO: 파쿠르 모션을 Chooser를 통해 배열로 리턴

	return Result;
}

UAnimMontage* UParkourTraversalComponent::MotionMatchTraversal(const TArray<UAnimMontage*>& ValidMontages) const
{
	// TODO: 모션 매칭 수행하기
	return nullptr;
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