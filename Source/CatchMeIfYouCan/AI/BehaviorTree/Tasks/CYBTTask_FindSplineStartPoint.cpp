// Fill out your copyright notice in the Description page of Project Settings.


#include "CYBTTask_FindSplineStartPoint.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"


UCYBTTask_FindSplineStartPoint::UCYBTTask_FindSplineStartPoint()
{
	NodeName = "Find Spline Start Point";

	//지정된 타입의 키만 보이도록 필터링
	PatrolPathActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UCYBTTask_FindSplineStartPoint, PatrolPathActorKey), AActor::StaticClass());
	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UCYBTTask_FindSplineStartPoint, TargetLocationKey));
}

EBTNodeResult::Type UCYBTTask_FindSplineStartPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//행동트리로 부터 블랙보드 가져오기
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindSplineStartPoint: 블랙보드를 가져오는데 실패 했습니다."));
		return EBTNodeResult::Failed;
	}

	//블랙보드로부터 순찰 경로 액터 가져오기
	AActor* PatrolPathActor = Cast<AActor>(BlackboardComp->GetValueAsObject(PatrolPathActorKey.SelectedKeyName));

	if (!PatrolPathActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindSplineStartPoint: 순찰 액터를 가져오는데 실패 했습니다."));
		return EBTNodeResult::Failed;
	}

	//액터에 스플라인 컴포넌트 있는지 확인
	USplineComponent* PatrolSpline = PatrolPathActor->FindComponentByClass<USplineComponent>();
	if (!PatrolSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindSplineStartPoint: 액터에서 스플라인을 찾지 못했습니다."));
		return EBTNodeResult::Failed;
	}

	//스플라인 시작 위치 가져와서 블랙보드에 저장
	const FVector StartPointLocation = PatrolSpline->GetLocationAtSplinePoint(0.0f, ESplineCoordinateSpace::World);
	BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, StartPointLocation);
	
	return EBTNodeResult::Succeeded;
}