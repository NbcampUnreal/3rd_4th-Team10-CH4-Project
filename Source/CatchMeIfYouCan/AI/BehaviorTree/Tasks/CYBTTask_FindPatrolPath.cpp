// Fill out your copyright notice in the Description page of Project Settings.


#include "CYBTTask_FindPatrolPath.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/CYAIDogCharacter.h"


UCYBTTask_FindPatrolPath::UCYBTTask_FindPatrolPath()
{
	// 행동 트리에서 보일 이름 설정
	NodeName = "Find Patrol Path And Set To Blackboard";

	// 블랙보드 키 필터 설정
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UCYBTTask_FindPatrolPath, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UCYBTTask_FindPatrolPath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// AI 컨트롤러와 개 캐릭터 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	ACYAIDogCharacter* DogCharacter = AIController ? Cast<ACYAIDogCharacter>(AIController->GetPawn()) : nullptr;

	if (!AIController || !DogCharacter)
	{
		return EBTNodeResult::Failed;
	}

	// 캐릭터에 설정된 순찰 경로 가져오기
	AActor* PatrolPathActor = DogCharacter->TargetPatrolPath;

	if (PatrolPathActor)
	{
		// 블랙보드의 지정된 키에 경로 액터 저장
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), PatrolPathActor);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}