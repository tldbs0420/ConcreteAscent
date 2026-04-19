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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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
	FTraversalChooserInputs CurrentTraversalInputs;

public:
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	AParkourObstacleBase* DetectObstacle();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	FTraversalChooserInputs EvaluateTraversal(AParkourObstacleBase* ObstacleBase);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool StartTraversal();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool StartLedgeGrab(float Direction = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void MoveAlongLedge(float Direction);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void ClimbFromLedge();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void DropFromLedge();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	FTransform BuildWarpTarget(const FTraversalChooserInputs& Inputs) const;

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	TArray<UAnimMontage*> BuildValidTraversalMontages(const FTraversalChooserInputs& Inputs) const;

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	UAnimMontage* MotionMatchTraversal(const TArray<UAnimMontage*>& ValidMontages) const;

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void PlaySelectedTraversalMontage(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void PlayLedgeMontage(ETraversalAction Action);
};
