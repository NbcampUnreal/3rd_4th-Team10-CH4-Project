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

// ===== TickTask 함수 내용을 새로운 로직으로 전체 교체 =====
void UCYBTTask_FollowSplinePath::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    ACharacter* ControlledCharacter = AIController ? Cast<ACharacter>(AIController->GetPawn()) : nullptr;
    if (!AIController || !ControlledCharacter || !BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    AActor* PatrolPathActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("PatrolPathActor")));
    if (!PatrolPathActor)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    USplineComponent* PatrolSpline = PatrolPathActor->FindComponentByClass<USplineComponent>();
    if (!PatrolSpline)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    float CurrentDistance = BlackboardComp->GetValueAsFloat(TEXT("CurrentSplineDistance"));
    float OldDistance = CurrentDistance;
    
    bool bOnSpline = BlackboardComp->GetValueAsBool(TEXT("bOnSpline"));
    
    FVector TargetLocation = PatrolSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
    FVector Direction = TargetLocation - ControlledCharacter->GetActorLocation();
    float DistanceToTarget = Direction.Size();
    
    // 한번이라도 스플라인 근처(50유닛)에 도달하면 bOnSpline = true로 설정
    if (!bOnSpline && DistanceToTarget < 50.0f)
    {
        bOnSpline = true;
        BlackboardComp->SetValueAsBool(TEXT("bOnSpline"), true);
    }
    
    // 스플라인에 한번 도달한 후에는 계속 스플라인 경로를 따라 이동
    if (bOnSpline)
    {
        CurrentDistance += PatrolSpeed * DeltaSeconds;
        float SplineLength = PatrolSpline->GetSplineLength();
        
        if (CurrentDistance >= SplineLength)
        {
            CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
        }
        
        TargetLocation = PatrolSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        Direction = TargetLocation - ControlledCharacter->GetActorLocation();
        DistanceToTarget = Direction.Size();
    }

    // 실제 이동 처리
    if (DistanceToTarget > 5.0f)
    {
        Direction.Normalize();
        ControlledCharacter->AddMovementInput(Direction, 1.0f);
    }

    // NPC 회전 처리
    FRotator CurrentRotation = ControlledCharacter->GetActorRotation();
    FRotator NewRotation;
    
    if (bOnSpline)
    {
        // 스플라인에 도달 후: 스플라인의 방향을 바라봄
        const FRotator SplineRotation = PatrolSpline->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        NewRotation = FRotator(0.f, SplineRotation.Yaw, 0.f);
    }
    else
    {
        // 스플라인 도달 전: 실제 이동 방향을 바라봄
        if (DistanceToTarget > 5.0f)
        {
            FRotator MoveRotation = Direction.Rotation();
            NewRotation = FRotator(0.f, MoveRotation.Yaw, 0.f);
        }
        else
        {
            NewRotation = CurrentRotation;
        }
    }
    
    const FRotator InterpolatedRotation = FMath::RInterpTo(CurrentRotation, NewRotation, DeltaSeconds, 3.0f);
    ControlledCharacter->SetActorRotation(InterpolatedRotation);

    // AI 애니메이션 변수 업데이트
    ACYAIDogCharacter* DogCharacter = Cast<ACYAIDogCharacter>(ControlledCharacter);
    if (DogCharacter)
    {
        float RealSpeed = 0.0f;
        float DirectionAngle = 0.0f;
        
        if (bOnSpline)
        {
            // 스플라인 위에서의 속도/방향 계산
            float DistanceMoved = FMath::Abs(CurrentDistance - OldDistance);
            if (CurrentDistance < OldDistance && (OldDistance - CurrentDistance) > (PatrolSpline->GetSplineLength() * 0.5f))
            {
                DistanceMoved = (PatrolSpline->GetSplineLength() - OldDistance) + CurrentDistance;
            }
            RealSpeed = DistanceMoved / DeltaSeconds;
            
            FVector SplineDirection = PatrolSpline->GetDirectionAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
            SplineDirection.Normalize();
            
            FVector CharacterForward = ControlledCharacter->GetActorForwardVector();
            FVector CharacterRight = ControlledCharacter->GetActorRightVector();
            
            if (RealSpeed > 5.0f)
            {
                float ForwardDot = FVector::DotProduct(SplineDirection, CharacterForward);
                float RightDot = FVector::DotProduct(SplineDirection, CharacterRight);
                DirectionAngle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
            }
        }
        else
        {
            // 스플라인 도달 전의 속도/방향 계산
            if (DistanceToTarget > 5.0f)
            {
                RealSpeed = PatrolSpeed;
                DirectionAngle = 0.0f; // 정면으로 전진
            }
        }
        
        if (RealSpeed < 5.0f || DistanceToTarget < 5.0f)
        {
            RealSpeed = 0.0f; // 대기 상태
        }
        
        // 애니메이션 변수 최종 업데이트
        float ScaledSpeed = FMath::Clamp(RealSpeed * 1.5f, 0.0f, 200.0f);
        DogCharacter->UpdateAIAnimationVariables(ScaledSpeed, DirectionAngle);
    }

    BlackboardComp->SetValueAsFloat(TEXT("CurrentSplineDistance"), CurrentDistance);
}