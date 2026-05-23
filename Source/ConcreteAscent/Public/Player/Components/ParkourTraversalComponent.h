// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ConcreteAscentTypes.h"
#include "ParkourTraversalComponent.generated.h"

class AConcreteAscentCharacter;
class AParkourObstacleBase;
class ALedgeObstacle;
class UParkourMotionData;
class UAnimMontage;
class UMotionWarpingComponent;

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CONCRETEASCENT_API UParkourTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParkourTraversalComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<AConcreteAscentCharacter> OwnerCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UParkourMotionData> MotionData;

	UPROPERTY(Transient)
	TObjectPtr<AParkourObstacleBase> DetectedObstacle;

	UPROPERTY(Transient)
	TObjectPtr<ALedgeObstacle> CurrentLedge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float CurrentLedgeOffset = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FTraversalCheckResult CurrentTraversalResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FTraversalChooserInputs CurrentTraversalInputs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FTraversalChooserOutputs CurrentTraversalOutputs;

	UPROPERTY(Transient)
	FHitResult LastObstacleHit;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAnimMontage>> CurrentValidMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	TEnumAsByte<ECollisionChannel> TraversableTraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	TEnumAsByte<ECollisionChannel> RoomTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float TraceForwardDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float TraceOriginZOffset = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float LedgeRoomPadding = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float FloorCheckExtraDistance = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|PoseSearch")
	float BranchInEndStartOffset = 0.03f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|MotionWarping")
	FName TraversalWarpTargetName = TEXT("TraversalTarget");

	bool CapsuleSweep(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const;
	bool HasCapsuleRoom(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const;

	FTraversalCheckResult BuildTraversalCheckResult(AParkourObstacleBase* ObstacleBase);

	FTraversalChooserInputs MakeChooserInputsFromCheckResult(
		const FTraversalCheckResult& CheckResult
	) const;

	FTransform BuildWarpTargetFromCheckResult(
		const FTraversalCheckResult& CheckResult
	) const;

	void ApplyTraversalWarpTarget(const FTransform& WarpTarget);

public:
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	AParkourObstacleBase* DetectObstacle();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	FTraversalChooserInputs EvaluateTraversal(AParkourObstacleBase* ObstacleBase);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool StartTraversal(FTraversalCheckResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool StartLedgeGrab(float Direction = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void MoveAlongLedge(float Direction);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void ClimbFromLedge();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void DropFromLedge();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	TArray<UAnimMontage*> BuildValidTraversalMontages(const FTraversalChooserInputs& Inputs, FTraversalChooserOutputs& OutOutputs);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool MotionMatchTraversal(const TArray<UAnimMontage*>& ValidMontages, UAnimMontage*& OutMontage, float& OutStartTime, float& OutPlayRate) const;

	bool FindPoseSearchBranchInEndTime(const UAnimMontage* Montage,float& OutEndTime) const;

	float GetStartTimeFromBranchInEnd(const UAnimMontage* Montage) const;

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void PlaySelectedTraversalMontage(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void PlayLedgeMontage(ETraversalAction Action);

	UFUNCTION(BlueprintImplementableEvent, Category = "Traversal")
	void BP_UpdatePoseSearchPlayerActor(AActor* PlayerActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Traversal")
	void BP_EvaluateTraversalChooser(const FTraversalChooserInputs& Inputs,  FTraversalChooserOutputs& Outputs, TArray<UAnimMontage*>& OutMontages);

	UFUNCTION(BlueprintImplementableEvent, Category = "Traversal")
	void BP_MotionMatchTraversal(const TArray<UAnimMontage*>& ValidMontages, UAnimMontage*& OutMontage, float& OutStartTime, float& OutPlayRate) const;
};
