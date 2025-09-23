// Fill out your copyright notice in the Description page of Project Settings.


#include "CYBTTask_SetIdleAnimation.h"
#include "AIController.h"
#include "AI/Characters/CYAIDogCharacter.h"

UCYBTTask_SetIdleAnimation::UCYBTTask_SetIdleAnimation()
{
	// 행동 트리에서 보일 이름 설정
	NodeName = "Set Idle Animation";
}

EBTNodeResult::Type UCYBTTask_SetIdleAnimation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// AI 컨트롤러 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	// 개 캐릭터 가져오기
	ACYAIDogCharacter* DogCharacter = Cast<ACYAIDogCharacter>(AIController->GetPawn());
	if (!DogCharacter)
	{
		return EBTNodeResult::Failed;
	}

	// 애니메이션 변수를 0으로 설정하여 아이들 상태로 전환
	DogCharacter->UpdateAIAnimationVariables(0.0f, 0.0f);

	return EBTNodeResult::Succeeded;
}
