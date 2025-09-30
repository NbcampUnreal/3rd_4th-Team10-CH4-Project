// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CYBTTask_SetIsOnSpline.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYBTTask_SetIsOnSpline : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UCYBTTask_SetIsOnSpline();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	//값을 변경할 블랙보드 키
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector IsOnSplineKey;

	//스플라인 위에 있는지
	UPROPERTY(EditAnywhere, Category="Settings")
	bool bValueToSet = true;
};
