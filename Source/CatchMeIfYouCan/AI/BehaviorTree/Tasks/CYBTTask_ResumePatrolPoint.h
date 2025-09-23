#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CYBTTask_ResumePatrolPoint.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API UCYBTTask_ResumePatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
    
public:
	UCYBTTask_ResumePatrolPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// 순찰 경로 액터가 담긴 블랙보드 키 저장
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolPathActorKey;

	// 계산된 거리를 저장할 블랙보드 키를 지정
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SplineDistanceKey;
};