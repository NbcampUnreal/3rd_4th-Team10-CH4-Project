// Fill out your copyright notice in the Description page of Project Settings.


#include "CYBTTask_FollowSplinePath.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/CYAIDogCharacter.h"

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

void UCYBTTask_FollowSplinePath::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 컨트롤러 가져오기 및 블랙보드 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	// 유효성 검사
	ACharacter* ControlledCharacter = AIController ? Cast<ACharacter>(AIController->GetPawn()) : nullptr;
	if (!AIController || !ControlledCharacter || !BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
    
	// 블랙보드에서 순찰 경로 액터 가져오기
	AActor* PatrolPathActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("PatrolPathActor")));
	if (!PatrolPathActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
    
	// 스플라인 컴포넌트 가져오기
	USplineComponent* PatrolSpline = PatrolPathActor->FindComponentByClass<USplineComponent>();
	if (!PatrolSpline)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	// 현재 스플라인상 위치 가져오기 및 업데이트
	float CurrentDistance = BlackboardComp->GetValueAsFloat(TEXT("CurrentSplineDistance"));
	float OldDistance = CurrentDistance; // 이전 거리 저장
    
	// 시간에 따른 거리 증가
	CurrentDistance += PatrolSpeed * DeltaSeconds;

	// 스플라인 끝에 도달하면 처음으로 루프
	float SplineLength = PatrolSpline->GetSplineLength();
	if (CurrentDistance >= SplineLength)
	{
		CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
	}

	// 계산된 거리에 해당하는 월드 위치 가져오기
	FVector TargetLocation = PatrolSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
	FVector Direction = TargetLocation - ControlledCharacter->GetActorLocation();
    
	//너무 가까우면 이동하지 않음
	float DistanceToTarget = Direction.Size();
	if (DistanceToTarget > 5.0f) // 5유닛보다 멀 때만 이동
	{
		Direction.Normalize();
		ControlledCharacter->AddMovementInput(Direction, 1.0f);
	}

	// NPC 회전 처리
	const FRotator SplineRotation = PatrolSpline->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
	const FRotator CurrentRotation = ControlledCharacter->GetActorRotation();
	FRotator NewRotation = FRotator(0.f, SplineRotation.Yaw, 0.f); // Yaw만 사용
    
	// 부드러운 회전을 위한 보간
	const FRotator InterpolatedRotation = FMath::RInterpTo(CurrentRotation, NewRotation, DeltaSeconds, 3.0f);
	ControlledCharacter->SetActorRotation(InterpolatedRotation);

	
	// AI 애니메이션 변수 업데이트
    ACYAIDogCharacter* DogCharacter = Cast<ACYAIDogCharacter>(ControlledCharacter);
    if (DogCharacter)
    {
        // 실제 이동 거리 계산
        float DistanceMoved = FMath::Abs(CurrentDistance - OldDistance);
        
        // 스플라인 끝에서 처음으로 넘어간 경우 처리
        if (CurrentDistance < OldDistance && (OldDistance - CurrentDistance) > (SplineLength * 0.5f))
        {
            DistanceMoved = (SplineLength - OldDistance) + CurrentDistance;
        }
        
        // 초당 실제 이동 속도 계산
        float RealSpeed = DistanceMoved / DeltaSeconds;
        
        // 최소 임계값 적용 (너무 작은 움직임은 아이들로 처리)
        if (RealSpeed < 5.0f || DistanceToTarget < 5.0f)
        {
            RealSpeed = 0.0f; // 아이들 상태
        }
        
        // 스플라인의 현재 지점에서의 방향 벡터 구하기
        FVector SplineDirection = PatrolSpline->GetDirectionAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        SplineDirection.Normalize();
        
        // 캐릭터 기준 방향 계산 (로컬 좌표계 변환)
        FVector CharacterForward = ControlledCharacter->GetActorForwardVector();
        FVector CharacterRight = ControlledCharacter->GetActorRightVector();
        
        float DirectionAngle = 0.0f;
        if (RealSpeed > 5.0f) // 움직이고 있을 때만 방향 계산
        {
            // 스플라인 방향을 캐릭터 로컬 좌표로 변환
            float ForwardDot = FVector::DotProduct(SplineDirection, CharacterForward);
            float RightDot = FVector::DotProduct(SplineDirection, CharacterRight);
            
            DirectionAngle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
        }
        
        // AI 애니메이션 변수 업데이트 (네트워크 복제됨)
        float ScaledSpeed = FMath::Clamp(RealSpeed * 1.5f, 0.0f, 200.0f);
        DogCharacter->UpdateAIAnimationVariables(ScaledSpeed, DirectionAngle);
        
 
    }

    // 업데이트된 거리를 블랙보드에 저장
    BlackboardComp->SetValueAsFloat(TEXT("CurrentSplineDistance"), CurrentDistance);
}