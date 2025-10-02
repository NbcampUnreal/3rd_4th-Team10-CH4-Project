#include "CYBTTask_FindSplineStartPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"

UCYBTTask_FindSplineStartPoint::UCYBTTask_FindSplineStartPoint()
{
	NodeName = "Find Spline Start Point";

	// --- ✨ 바로 이 부분이 핵심 해결책입니다! ---
	// 에디터에게 PatrolPathActorKey 변수에는 'Actor' 타입의 키만 보여달라고 알려줍니다.
	PatrolPathActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UCYBTTask_FindSplineStartPoint, PatrolPathActorKey), AActor::StaticClass());

	// 에디터에게 TargetLocationKey 변수에는 'Vector' 타입의 키만 보여달라고 알려줍니다.
	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UCYBTTask_FindSplineStartPoint, TargetLocationKey));
	// ------------------------------------
}

EBTNodeResult::Type UCYBTTask_FindSplineStartPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	// 1. 블랙보드에서 순찰 경로 액터(스플라인)를 가져옵니다.
	UObject* PatrolPathObject = BlackboardComp->GetValueAsObject(PatrolPathActorKey.SelectedKeyName);
	AActor* PatrolPathActor = Cast<AActor>(PatrolPathObject);
	if (!PatrolPathActor)
	{
		// 순찰 경로가 없으면 실패 처리
		return EBTNodeResult::Failed;
	}

	// 2. 액터에서 스플라인 컴포넌트를 찾습니다.
	USplineComponent* PatrolSpline = PatrolPathActor->FindComponentByClass<USplineComponent>();
	if (!PatrolSpline)
	{
		// 스플라인 컴포넌트가 없으면 실패 처리
		return EBTNodeResult::Failed;
	}

	// 3. 스플라인의 시작점(인덱스 0)의 월드 좌표를 가져옵니다.
	const FVector StartPointLocation = PatrolSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

	// 4. 계산된 시작점 좌표를 TargetLocationKey가 가리키는 블랙보드 키에 저장합니다.
	BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, StartPointLocation);

	// 성공적으로 계산 및 저장을 완료했음을 알립니다.
	return EBTNodeResult::Succeeded;
}
