// Fill out your copyright notice in the Description page of Project Settings.


#include "CYBTTask_SetIsOnSpline.h"

#include "BehaviorTree/BlackboardComponent.h"

UCYBTTask_SetIsOnSpline::UCYBTTask_SetIsOnSpline()
{
	NodeName= "Set IsOnSpline";

	//불타입 키만 사용
	IsOnSplineKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UCYBTTask_SetIsOnSpline, IsOnSplineKey));
}

EBTNodeResult::Type UCYBTTask_SetIsOnSpline::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		// 에디터에서 설정한 'bValueToSet' 값으로 블랙보드 키의 값을 변경
		BlackboardComp->SetValueAsBool(IsOnSplineKey.SelectedKeyName, bValueToSet);

		// 태스크 성공
		return EBTNodeResult::Succeeded;
	}

	// 블랙보드가 없으면 실패 처리합니다.
	return EBTNodeResult::Failed;
}
