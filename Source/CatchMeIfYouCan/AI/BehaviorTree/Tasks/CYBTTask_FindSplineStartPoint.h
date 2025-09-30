// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CYBTTask_FindSplineStartPoint.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYBTTask_FindSplineStartPoint : public UBTTask_BlackboardBase
{
	GENERATED_BODY()


public:
	UCYBTTask_FindSplineStartPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:

	//행동트리에서 어떤 블랙보드 변수를 읽어올지 지정
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "Patrol Path Actor Key"))
	FBlackboardKeySelector PatrolPathActorKey;

	//시작점의 위치를 저장할 블랙보드키
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "Target Location Key"))
	FBlackboardKeySelector TargetLocationKey;
	
};
