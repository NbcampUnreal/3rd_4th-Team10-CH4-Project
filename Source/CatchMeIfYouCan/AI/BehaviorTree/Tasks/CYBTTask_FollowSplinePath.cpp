#include "CYBTTask_FollowSplinePath.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/CYAIDogCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UCYBTTask_FollowSplinePath::UCYBTTask_FollowSplinePath()
{
    // 행동 트리에서 보일 이름 설정
    NodeName = "Follow Spline Path";
    
    //틱 사용
    bNotifyTick = true;
}

EBTNodeResult::Type UCYBTTask_FollowSplinePath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    //InProgress를 반환하여 계속 실행되도록 함
    return EBTNodeResult::InProgress;
}

// ===== TickTask 함수 내용을 새로운 로직으로 전체 교체 =====
void UCYBTTask_FollowSplinePath::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
  AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	
	// 필수 컴포넌트들이 하나라도 없으면 태스크를 실패 처리하고 즉시 종료합니다.
	if (!AIController || !BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACharacter* ControlledCharacter = Cast<ACharacter>(AIController->GetPawn());
	if (!ControlledCharacter)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	// 블랙보드에서 순찰 경로 액터를 가져옵니다.
	AActor* PatrolPathActor = Cast<AActor>(BlackboardComp->GetValueAsObject(GetSelectedBlackboardKey()));
	USplineComponent* PatrolSpline = PatrolPathActor ? PatrolPathActor->FindComponentByClass<USplineComponent>() : nullptr;
	
	if (!PatrolSpline)
	{
		// 스플라인이 유효하지 않으면 태스크 실패
		UE_LOG(LogTemp, Warning, TEXT("%s: Patrol Spline not found."), *GetNodeName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}



	// 1. 블랙보드에서 현재까지 이동한 거리를 가져옵니다.
	float CurrentDistance = BlackboardComp->GetValueAsFloat(TEXT("CurrentSplineDistance"));

	// 2. 이번 프레임에 이동할 거리를 계산합니다.
	CurrentDistance += PatrolSpeed * DeltaSeconds;
	const float SplineLength = PatrolSpline->GetSplineLength();
	
	// 3. 스플라인 경로의 끝에 도달하면 처음으로 되돌립니다.
	if (CurrentDistance >= SplineLength)
	{
		CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
	}
	
	// 4. 계산된 거리의 스플라인 위 위치와 방향을 구합니다.
	const FVector TargetLocation = PatrolSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
	const FRotator TargetRotation = PatrolSpline->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);

	// 5. AI를 부드럽게 회전시킵니다.
	const FRotator NewRotation = FMath::RInterpTo(
		ControlledCharacter->GetActorRotation(),
		FRotator(0.f, TargetRotation.Yaw, 0.f), // Z축 회전(Yaw)만 사용
		DeltaSeconds,
		5.0f // 회전 속도
	);
	ControlledCharacter->SetActorRotation(NewRotation);

	// 6. 목표 위치로 이동 입력을 줍니다.
	const FVector Direction = TargetLocation - ControlledCharacter->GetActorLocation();
	ControlledCharacter->AddMovementInput(Direction.GetSafeNormal());
	
	// 7. (선택사항) 애니메이션을 위해 속도와 방향 정보를 업데이트합니다.
	if (ACYAIDogCharacter* DogCharacter = Cast<ACYAIDogCharacter>(ControlledCharacter))
	{
		DogCharacter->UpdateAIAnimationVariables(PatrolSpeed, 0.f); // 스플라인 정방향이므로 방향은 0
	}

	// 8. 계산된 최종 이동 거리를 다시 블랙보드에 저장합니다.
	BlackboardComp->SetValueAsFloat(TEXT("CurrentSplineDistance"), CurrentDistance);
}

EBTNodeResult::Type UCYBTTask_FollowSplinePath::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 이 태스크가 다른 행동(예: 플레이어 추격)에 의해 중단될 때 호출됩니다.
	// AI의 움직임을 즉시 멈추고 애니메이션을 초기화하여 자연스러운 전환을 만듭니다.
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		if (ACharacter* Character = Cast<ACharacter>(AIController->GetPawn()))
		{
			Character->GetCharacterMovement()->StopMovementImmediately();

			if (ACYAIDogCharacter* DogCharacter = Cast<ACYAIDogCharacter>(Character))
			{
				//경비견의 움직임도 멈춤
				DogCharacter->UpdateAIAnimationVariables(0.0f, 0.0f);
			}
		}
	}
	return EBTNodeResult::Aborted;
}
