#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CYBTTask_FollowSplinePath.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API UCYBTTask_FollowSplinePath : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UCYBTTask_FollowSplinePath();

protected:
	// 태스크가 시작될 때 한 번 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 태스크가 활성화된 동안 매 프레임 호출되어 스플라인 경로를 따라 이동
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	//행동 트리의 다른 블랜치가 활성화되어 이 태스크가 종료될때 호출
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolSpeed = 300.0f;
};