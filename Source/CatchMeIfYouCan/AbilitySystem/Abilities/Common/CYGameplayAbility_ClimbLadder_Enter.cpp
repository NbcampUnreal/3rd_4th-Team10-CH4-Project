// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_ClimbLadder_Enter.h"

#include "CYLogChannels.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_WaitForLadderExit.h"
#include "Actors/CYLadderBase.h"
#include "Character/Components/CYCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"

UCYGameplayAbility_ClimbLadder_Enter::UCYGameplayAbility_ClimbLadder_Enter()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    ActivationPolicy = ECYAbilityActivationPolicy::OnInputTriggered;
    
    // TODO: 태그 설정 (필요시 추가)
    // AbilityTags.AddTag(CYGameplayTags::Ability_Movement_ClimbLadder_Enter);
    // ActivationOwnedTags.AddTag(CYGameplayTags::Status_Movement_Ladder);
}

void UCYGameplayAbility_ClimbLadder_Enter::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    CharacterMovementComponent = Cast<UCYCharacterMovementComponent>(Character->GetCharacterMovement());
    if (!CharacterMovementComponent)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    // 사다리 탐색 및 파라미터 결정
    ACYLadderBase* TargetLadder = nullptr;
    FVector LadderBottom, LadderTop, LadderFacing;
    float InitialRailParameter = -1.0f;
    bool bIsClimbingUp = true;

    if (!FindAndValidateLadder(TargetLadder, LadderBottom, LadderTop, LadderFacing, InitialRailParameter, bIsClimbingUp, TriggerEventData))
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    // 사다리 등반 시작
    CharacterMovementComponent->BeginClimbLadder(TargetLadder, LadderBottom, LadderTop, LadderFacing, InitialRailParameter);

    // 이탈 감시 태스크 시작
    constexpr float CheckRate = 0.02f; // 50Hz 체크 주기
    UCYAbilityTask_WaitForLadderExit* ExitMonitorTask = UCYAbilityTask_WaitForLadderExit::CreateWaitForLadderExitTask(this, TargetLadder, LadderBottom, LadderTop, LadderFacing, CheckRate);
    
    if (ExitMonitorTask)
    {
        ExitMonitorTask->OnExitTop.AddDynamic(this, &ThisClass::HandleLadderExitFromTop);
        ExitMonitorTask->OnExitBottom.AddDynamic(this, &ThisClass::HandleLadderExitFromBottom);
        ExitMonitorTask->OnCancelled.AddDynamic(this, &ThisClass::HandleLadderClimbingCancelled);
        ExitMonitorTask->ReadyForActivation();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ClimbLadder_Enter: Failed to create exit monitor task"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderExitFromTop()
{
    if (CharacterMovementComponent)
    {
        CharacterMovementComponent->EndClimbLadder(/*bStepOffTop=*/true);
        UE_LOG(LogTemp, Log, TEXT("ClimbLadder_Enter: Exited from top"));
    }
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderExitFromBottom()
{
    if (CharacterMovementComponent)
    {
        CharacterMovementComponent->EndClimbLadder(/*bStepOffTop=*/false);
        UE_LOG(LogTemp, Log, TEXT("ClimbLadder_Enter: Exited from bottom"));
    }
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderClimbingCancelled()
{
    if (CharacterMovementComponent)
    {
        CharacterMovementComponent->EndClimbLadder(/*bStepOffTop=*/false);
        UE_LOG(LogTemp, Log, TEXT("ClimbLadder_Enter: Climbing cancelled"));
    }
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

bool UCYGameplayAbility_ClimbLadder_Enter::FindAndValidateLadder(ACYLadderBase*& OutLadderActor,FVector& OutBottomLocation,FVector& OutTopLocation,FVector& OutFacingDirection, float& OutInitialRailParameter, bool& OutIsClimbingUp, const FGameplayEventData* EventData) const
{
    const AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar)
    {
        return false;
    }

    ACYLadderBase* FoundLadder = nullptr;
    
    // 1단계: 이벤트 데이터에서 타겟 사다리 확인 (상호작용 시스템)
    if (EventData && EventData->Target)
    {
        FoundLadder = Cast<ACYLadderBase>(const_cast<AActor*>(EventData->Target.Get()));
    }

    // 2단계: 이벤트에 사다리가 없으면 주변 검색
    if (!FoundLadder)
    {
        FoundLadder = FindNearestLadderInRange(Avatar->GetActorLocation());
    }

    // 3단계: 사다리를 찾지 못함
    if (!FoundLadder)
    {
        return false;
    }

    // 4단계: 사다리 사용 가능 여부 검증 (거리, 각도)
    if (!FoundLadder->CanCharacterUseLadder(Avatar, MaximumEntryDistance, MaximumEntryAngleDegrees))
    {
        UE_LOG(LogCY, Warning, TEXT("FindAndValidateLadder: Ladder found but not usable (distance or angle)"));
        return false;
    }

    // 5단계: 등반 방향 결정 (캐릭터 입력 기반)
    const ACharacter* Character = Cast<ACharacter>(Avatar);
    OutIsClimbingUp = Character ? DetermineClimbingDirection(Character, FoundLadder) : true;

    // 6단계: 사다리 정보 및 초기 위치 설정
    OutBottomLocation = FoundLadder->GetBottomWorldLocation();
    OutTopLocation = FoundLadder->GetTopWorldLocation();
    OutFacingDirection = FoundLadder->GetHorizontalFacingDirection();
    OutInitialRailParameter = FoundLadder->CalculateEntryRailParameter(OutIsClimbingUp, EdgeEntryOffset);

    OutLadderActor = FoundLadder;
    return true;
}

ACYLadderBase* UCYGameplayAbility_ClimbLadder_Enter::FindNearestLadderInRange(const FVector& SearchOrigin) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // 구체 오버랩으로 주변 액터 검색
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FindNearestLadder), false);

    const bool bHasOverlaps = World->OverlapMultiByChannel(
        OverlapResults,
        SearchOrigin,
        FQuat::Identity,
        LadderDetectionChannel,
        FCollisionShape::MakeSphere(LadderSearchRadius),
        QueryParams
    );

    if (!bHasOverlaps)
    {
        return nullptr;
    }

    // 가장 가까운 사다리 찾기
    ACYLadderBase* NearestLadder = nullptr;
    float NearestDistanceSquared = TNumericLimits<float>::Max();

    for (const FOverlapResult& Result : OverlapResults)
    {
        ACYLadderBase* Ladder = Cast<ACYLadderBase>(Result.GetActor());
        if (!Ladder)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared2D(SearchOrigin, Ladder->GetActorLocation());
        if (DistanceSquared < NearestDistanceSquared)
        {
            NearestDistanceSquared = DistanceSquared;
            NearestLadder = Ladder;
        }
    }
    
    return NearestLadder;
}

bool UCYGameplayAbility_ClimbLadder_Enter::DetermineClimbingDirection(const ACharacter* Character, const ACYLadderBase* Ladder) const
{
    if (!Character || !Ladder)
    {
        return true;
    }
    const FVector InputDirection = Character->GetLastMovementInputVector().GetSafeNormal2D();
    const float   InputMagnitude = Character->GetLastMovementInputVector().Size2D();

    // 입력이 거의 없으면 진입 안 함(임계치 0.2~0.35 사이 권장)
    constexpr float keyInputThreshold = 0.25f;
    if (InputMagnitude < keyInputThreshold)
    {
        return true;
    }
    
    // 사다리 정면과 입력 방향의 내적: +면 사다리 정면쪽(상향), -면 반대쪽(하향)
    const FVector LadderFacingDirection = Ladder->GetHorizontalFacingDirection();
    const FVector CharacterFacingDirectionOnLadder = -LadderFacingDirection;
    
    const float Dot = FVector::DotProduct(InputDirection, CharacterFacingDirectionOnLadder);

    return Dot >= 0.0f; // true: 상향, false: 하향
}