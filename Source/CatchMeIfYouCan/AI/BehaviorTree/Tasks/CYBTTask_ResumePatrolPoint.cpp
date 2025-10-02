// Fill out your copyright notice in the Description page of Project Settings.


#include "CYBTTask_ResumePatrolPoint.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"

UCYBTTask_ResumePatrolPoint::UCYBTTask_ResumePatrolPoint()
{
	// 행동 트리에서 보일 이름 설정
	NodeName = "Resume Patrol From Closest Point";
}

EBTNodeResult::Type UCYBTTask_ResumePatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 필요한 컴포넌트들 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	ACharacter* ControlledCharacter = AIController ? Cast<ACharacter>(AIController->GetPawn()) : nullptr;

	// 유효성 검사
	if (!AIController || !ControlledCharacter || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	// 블랙보드에서 순찰 경로 액터 가져오기
	AActor* PatrolPathActor = Cast<AActor>(BlackboardComp->GetValueAsObject(PatrolPathActorKey.SelectedKeyName));
	if (!PatrolPathActor)
	{
		return EBTNodeResult::Failed;
	}

	// 순찰 경로 액터에서 스플라인 컴포넌트 찾기
	USplineComponent* PatrolSpline = PatrolPathActor->FindComponentByClass<USplineComponent>();
	if (!PatrolSpline)
	{
		return EBTNodeResult::Failed;
	}

	// 현재 캐릭터 위치에서 스플라인상 가장 가까운 지점의 InputKey 찾기
	const FVector CurrentLocation = ControlledCharacter->GetActorLocation();
	const float ClosestInputKey = PatrolSpline->FindInputKeyClosestToWorldLocation(CurrentLocation);
    
	// 그 지점의 스플라인상 '거리(Distance)' 계산
	const float NewDistance = PatrolSpline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);

	// --- ✨ 여기가 업그레이드된 부분입니다! ---
	// 그 지점의 '월드 좌표(Location)'도 계산합니다.
	const FVector ClosestLocationOnSpline = PatrolSpline->GetLocationAtSplineInputKey(ClosestInputKey, ESplineCoordinateSpace::World);
	
	// 계산된 '월드 좌표'를 'TargetLocation' 키에 저장하여 Move To 태스크가 사용하도록 합니다.
	BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), ClosestLocationOnSpline);
	// ------------------------------------

	// 계산된 '거리'를 블랙보드에 저장하여 Follow Spline Path 태스크가 사용하도록 합니다.
	BlackboardComp->SetValueAsFloat(SplineDistanceKey.SelectedKeyName, NewDistance);

	return EBTNodeResult::Succeeded;
}