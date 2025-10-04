// Fill out your copyright notice in the Description page of Project Settings.


#include "CYAbilityTask_WaitForLadderExit.h"

#include "Character/Components/CYCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UCYAbilityTask_WaitForLadderExit* UCYAbilityTask_WaitForLadderExit::CreateWaitForLadderExitTask(UGameplayAbility* OwningAbility, AActor* LadderActor, FVector LadderBottomLocation, FVector LadderTopLocation, FVector LadderFacingDirection, float UpdateFrequency, float EdgeDetectionTolerance)
{
    // 새 태스크 인스턴스 생성
    UCYAbilityTask_WaitForLadderExit* NewTask = NewAbilityTask<UCYAbilityTask_WaitForLadderExit>(OwningAbility);
    
    // 사다리 정보 설정
    NewTask->MonitoredLadder = LadderActor;
    NewTask->LadderBottomWorldLocation = LadderBottomLocation;
    NewTask->LadderTopWorldLocation = LadderTopLocation;
    NewTask->LadderFacingDirection = LadderFacingDirection;
    
    // 파생 정보 계산
    NewTask->LadderClimbDirection = (LadderTopLocation - LadderBottomLocation).GetSafeNormal();
    NewTask->LadderTotalHeight = FVector::Distance(LadderBottomLocation, LadderTopLocation);
    
    // 설정 파라미터
    NewTask->CheckUpdateFrequency = FMath::Max(0.01f, UpdateFrequency); // 최소 100Hz
    NewTask->EdgeProximityThreshold = FMath::Max(1.0f, EdgeDetectionTolerance); // 최소 1cm
    
    return NewTask;
}

void UCYAbilityTask_WaitForLadderExit::Activate()
{
    Super::Activate();

    // Avatar가 준비될 때까지 대기
    SetWaitingOnAvatar();

    // 주기적 체크 타이머 시작
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PeriodicCheckTimer,
            this,
            &ThisClass::PerformExitConditionCheck,
            CheckUpdateFrequency,
            true // 반복
        );
    }
}

void UCYAbilityTask_WaitForLadderExit::OnDestroy(bool bInOwnerFinished)
{
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PeriodicCheckTimer);
    }
    
    Super::OnDestroy(bInOwnerFinished);
}

void UCYAbilityTask_WaitForLadderExit::PerformExitConditionCheck()
{
    // 캐릭터와 MovementComponent 유효성 검증
    ACharacter* Character = nullptr;
    UCYCharacterMovementComponent* MovementComponent = nullptr;
    
    if (!ValidateCharacterAndLadder(Character, MovementComponent))
    {
        OnCancelled.Broadcast();
        EndTask();
        return;
    }

    // 사다리 타기 상태 확인
    if (!MovementComponent->IsClimbingLadder())
    {
        OnCancelled.Broadcast();
        EndTask();
        return;
    }

    // 점프 입력 감지 (즉시 취소)
    if (Character->bPressedJump)
    {
        OnCancelled.Broadcast();
        EndTask();
        return;
    }

    // 현재 레일 위치 계산
    const float CurrentRailPosition = CalculateCharacterRailPosition(Character->GetActorLocation());
    
    // 상/하단 근접 여부 확인
    const bool bIsNearBottom = IsNearLadderEdge(CurrentRailPosition, false);
    const bool bIsNearTop = IsNearLadderEdge(CurrentRailPosition, true);
    
    // 입력 의도 분석 (상향/하향/중립)
    const float ClimbingIntent = AnalyzeClimbingIntent(Character);
    const bool bWantsToClimbUp = ClimbingIntent > INPUT_INTENT_THRESHOLD;
    const bool bWantsToClimbDown = ClimbingIntent < -INPUT_INTENT_THRESHOLD;

    // 이탈 조건 판단
    if (bIsNearTop && bWantsToClimbUp)
    {
        OnExitTop.Broadcast();
        EndTask();
    }
    else if (bIsNearBottom && bWantsToClimbDown)
    {
        OnExitBottom.Broadcast();
        EndTask();
    }
}

float UCYAbilityTask_WaitForLadderExit::CalculateCharacterRailPosition(const FVector& CharacterLocation) const
{
    // 캐릭터 위치를 레일에 투영
    const FVector RelativeToBottom = CharacterLocation - LadderBottomWorldLocation;
    const float ProjectedDistance = FVector::DotProduct(RelativeToBottom, LadderClimbDirection);
    
    // 0 ~ TotalHeight 범위로 클램프
    return FMath::Clamp(ProjectedDistance, 0.0f, LadderTotalHeight);
}

float UCYAbilityTask_WaitForLadderExit::AnalyzeClimbingIntent(const ACharacter* Character) const
{
    if (!Character)
    {
        return 0.0f;
    }
    
    const FVector InputDirection = Character->GetLastMovementInputVector().GetSafeNormal2D();
    const float   InputMagnitude   = Character->GetLastMovementInputVector().Size2D();

    // 너무 미세한 입력은 의도 없음 처리
    constexpr float KeyInputThreshold = 0.25f;
    if (InputMagnitude < KeyInputThreshold)
    {
        return 0.0f;
    }

    const FVector CharacterFacingOnLadder = -LadderFacingDirection;
    
    // 사다리 정면과의 내적: + 상향(정면), - 하향(후면)
    const float Dot = FVector::DotProduct(InputDirection, CharacterFacingOnLadder);
    return Dot; 
}

bool UCYAbilityTask_WaitForLadderExit::IsNearLadderEdge(float RailPosition, bool bCheckTop) const
{
    if (bCheckTop)
    {
        // 상단 체크: 전체 높이에서 임계값을 뺀 위치 이상
        return RailPosition >= (LadderTotalHeight - EdgeProximityThreshold);
    }
    else
    {
        // 하단 체크: 임계값 이하
        return RailPosition <= EdgeProximityThreshold;
    }
}

bool UCYAbilityTask_WaitForLadderExit::ValidateCharacterAndLadder(ACharacter*& OutCharacter, UCYCharacterMovementComponent*& OutMovementComponent) const
{
    // 어빌리티 유효성
    if (!Ability)
    {
        return false;
    }

    // 캐릭터 가져오기
    OutCharacter = Cast<ACharacter>(Ability->GetCurrentActorInfo()->AvatarActor.Get());
    if (!OutCharacter)
    {
        return false;
    }

    // MovementComponent 가져오기
    OutMovementComponent = Cast<UCYCharacterMovementComponent>(OutCharacter->GetCharacterMovement());
    if (!OutMovementComponent)
    {
        return false;
    }

    // 사다리 유효성
    if (!MonitoredLadder.IsValid())
    {
        return false;
    }

    return true;
}