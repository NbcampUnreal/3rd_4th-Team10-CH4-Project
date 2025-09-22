// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CYBTTask_SetIdleAnimation.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYBTTask_SetIdleAnimation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UCYBTTask_SetIdleAnimation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
