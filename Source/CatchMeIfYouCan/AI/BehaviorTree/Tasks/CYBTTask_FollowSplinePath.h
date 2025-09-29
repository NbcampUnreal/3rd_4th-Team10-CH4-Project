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

	// 태스크가 활성화된 동안 매 프레임 호출
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolSpeed = 300.0f;
};