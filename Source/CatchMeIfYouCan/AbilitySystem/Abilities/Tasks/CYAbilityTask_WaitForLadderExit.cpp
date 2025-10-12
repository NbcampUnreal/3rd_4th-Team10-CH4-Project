#include "CYAbilityTask_WaitForLadderExit.h"

#include "AbilitySystemComponent.h"
#include "Character/CYStatusGameplayTags.h"
#include "Character/Components/CYCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UCYAbilityTask_WaitForLadderExit* UCYAbilityTask_WaitForLadderExit::CreateWaitForLadderExitTask(
    UGameplayAbility* OwningAbility,
    AActor* LadderActor,
    FVector LadderBottomLocation,
    FVector LadderTopLocation,
    FVector LadderFacingDirection,
    float UpdateFrequency,
    float BottomEdgeThreshold,
    float TopEdgeThreshold,
    FVector EntryTargetLocation,
    float SafetyMargin)
{
    UCYAbilityTask_WaitForLadderExit* NewTask = NewAbilityTask<UCYAbilityTask_WaitForLadderExit>(OwningAbility);

    NewTask->MonitoredLadder = LadderActor;
    NewTask->LadderBottomWorldLocation = LadderBottomLocation;
    NewTask->LadderTopWorldLocation = LadderTopLocation;
    NewTask->LadderFacingDirection = LadderFacingDirection;
    NewTask->LadderClimbDirection = (LadderTopLocation - LadderBottomLocation).GetSafeNormal();
    NewTask->LadderTotalHeight = FVector::Distance(LadderBottomLocation, LadderTopLocation);
    NewTask->CheckUpdateFrequency = FMath::Max(0.01f, UpdateFrequency); 
    NewTask->BottomEdgeProximityThreshold = FMath::Max(1.0f, BottomEdgeThreshold);
    NewTask->TopEdgeProximityThreshold = FMath::Max(1.0f, TopEdgeThreshold);
    NewTask->EntryTargetLocation = EntryTargetLocation;
    NewTask->HorizontalDistanceSafetyMargin = FMath::Max(1.0f, SafetyMargin);
    
    return NewTask;
}

void UCYAbilityTask_WaitForLadderExit::Activate()
{
    Super::Activate();

    // Avatar가 준비될 때까지 대기
    SetWaitingOnAvatar();

    if (!EntryTargetLocation.IsZero())
    {
        const float BaseDistance = CalculateHorizontalDistanceFromRail(EntryTargetLocation);
        MaxAllowedHorizontalDistance = BaseDistance + HorizontalDistanceSafetyMargin;
    }

    // 점프 태그 이벤트 구독 (즉시 감지)
    if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
    {
        JumpTagDelegateHandle = ASC->RegisterGameplayTagEvent(
            CYGameplayTags::Status_Action_Jump,
            EGameplayTagEventType::NewOrRemoved  
        ).AddUObject(this, &UCYAbilityTask_WaitForLadderExit::OnJumpTagChanged);
    }
    
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
    // 점프 태그 델리게이트 해제
    if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
    {
        if (JumpTagDelegateHandle.IsValid())
        {
            ASC->RegisterGameplayTagEvent(
                CYGameplayTags::Status_Action_Jump,
                EGameplayTagEventType::NewOrRemoved
            ).Remove(JumpTagDelegateHandle);
            JumpTagDelegateHandle.Reset();
        }
    }
    
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

    const FVector CharacterLocation = Character->GetActorLocation();

    // 실제 거리 = 현재 캐릭터 위치로부터의 거리
    const float ActualDistance = CalculateHorizontalDistanceFromRail(CharacterLocation);
    
    if (ActualDistance > MaxAllowedHorizontalDistance)
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

void UCYAbilityTask_WaitForLadderExit::OnJumpTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    // TODO 몽타주 활성화 시 점프 로직 무시를 추후 개선된 방식으로 변경 예정
    ACharacter* Character = Cast<ACharacter>(Ability->GetCurrentActorInfo()->AvatarActor.Get());
    if (!Character)
    {
        return;
    }
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        if (AnimInstance->IsAnyMontagePlaying())
        {

            return;
        }
    }
    
    if (NewCount > 0)
    {
        OnCancelled.Broadcast();
        EndTask();
    }
}

float UCYAbilityTask_WaitForLadderExit::CalculateCharacterRailPosition(const FVector& CharacterLocation) const
{
    // 캐릭터 위치를 레일에 투영
    const FVector RelativeToBottom = CharacterLocation - LadderBottomWorldLocation;
    const float ProjectedDistance = FVector::DotProduct(RelativeToBottom, LadderClimbDirection);
    
    // 0 ~ TotalHeight 범위로 클램프
    //return FMath::Clamp(ProjectedDistance, 0.0f, LadderTotalHeight);

    return ProjectedDistance;
}

float UCYAbilityTask_WaitForLadderExit::AnalyzeClimbingIntent(const ACharacter* Character) const
{
    if (!Character)
    {
        return 0.0f;
    }

    UCYCharacterMovementComponent* MovementComp = Cast<UCYCharacterMovementComponent>(Character->GetCharacterMovement());
    if (!MovementComp)
    {
        return 0.0f;
    }

    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        if (AnimInstance->IsAnyMontagePlaying())
        {
            // 몽타주 재생 중 = 입력 무시
            return 0.0f;
        }
    }

    const FVector InputDirection = MovementComp->GetCurrentAcceleration();
    const float InputMagnitude = InputDirection.Size();
    
    if (InputMagnitude < INPUT_INTENT_THRESHOLD)
    {
        return 0.0f;
    }
    
    // 컨트롤러 Forward와 비교 (카메라 방향)
    AController* Controller = Character->GetController();
    if (!Controller)
    {
        return 0.0f;
    }
    const FRotator ControlRot = Controller->GetControlRotation();
    const FVector ControlForward = FRotationMatrix(FRotator(0.f, ControlRot.Yaw, 0.f)).GetUnitAxis(EAxis::X);
    
    // 입력을 컨트롤러 Forward에 투영
    const float Dot = FVector::DotProduct(InputDirection.GetSafeNormal(), ControlForward);

    return Dot;
}

bool UCYAbilityTask_WaitForLadderExit::IsNearLadderEdge(float RailPosition, bool bCheckTop) const
{
    if (bCheckTop)
    {
        // 상단 체크: 전체 높이에서 임계값을 뺀 위치 이상
        const float TopThreshold = LadderTotalHeight - TopEdgeProximityThreshold;
    
        return RailPosition >= TopThreshold;
    }
    // 하단 체크: 임계값 이하
    return RailPosition <= BottomEdgeProximityThreshold;
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

float UCYAbilityTask_WaitForLadderExit::CalculateHorizontalDistanceFromRail(const FVector& CharacterLocation) const
{
    const FVector ToCharacter = CharacterLocation - LadderBottomWorldLocation;
    const float ProjectedDistance = FVector::DotProduct(ToCharacter, LadderClimbDirection);
    
    // 투영 거리를 레일 범위로 클램핑 (안전장치)
    const float ClampedDistance = FMath::Clamp(ProjectedDistance, 0.0f, LadderTotalHeight);
    
    // 레일 상의 가장 가까운 지점
    const FVector ClosestPointOnRail = LadderBottomWorldLocation + (LadderClimbDirection * ClampedDistance);
    
    // 수평 거리 계산 (Z축 제외)
    const float HorizontalDistance = FVector::Dist2D(CharacterLocation, ClosestPointOnRail);
    
    return HorizontalDistance;
}
