// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_ClimbLadder_Enter.h"

#include "CYLogChannels.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "Character/CYStatusGameplayTags.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_WaitForLadderExit.h"
#include "Actors/CYLadderBase.h"
#include "Character/Components/CYCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UCYGameplayAbility_ClimbLadder_Enter::UCYGameplayAbility_ClimbLadder_Enter()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    
    // GameplayEvent로만 활성화 (상호작용 또는 자동 그랩)
    ActivationPolicy = ECYAbilityActivationPolicy::Manual; 
 
    AbilityTags.AddTag(CYGameplayTags::Ability_Action_Climbing);
    ActivationOwnedTags.AddTag(CYGameplayTags::Status_Movement_Climbing);

    ActivationBlockedTags.AddTag(CYGameplayTags::Status_Movement_Climbing);
    
    // TODO: 적절한 취소 태그
    // CancelAbilitiesWithTag.AddTag(CYGameplayTags::);
    
    // GameplayEvent 트리거 설정
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = CYGameplayTags::Ability_Action_Climbing;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);
}

void UCYGameplayAbility_ClimbLadder_Enter::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 캐릭터 검증
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    // MovementComponent 캐싱
    CachedMovementComponent = Cast<UCYCharacterMovementComponent>(Character->GetCharacterMovement());
    if (!CachedMovementComponent)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    // 이미 사다리 타는 중인지 체크
    if (CachedMovementComponent->IsClimbingLadder())
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }
    
    // 사다리 정보 추출
    ACYLadderBase* TempLadder = nullptr;
    FVector LadderBottom, LadderTop, LadderFacing;
    float LadderStandOff = 0.0f;
    float InitialRailParameter = 0.0f;
    bool bIsClimbingUp = true;

    if (!CanExtractLadderInfo(TriggerEventData, TempLadder, LadderBottom, LadderTop, LadderFacing, LadderStandOff, InitialRailParameter, bIsClimbingUp))
    {
        UE_LOG(LogCY, Warning, TEXT("ClimbLadder_Enter: Failed to extract ladder info"));
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    CurrentLadder = TempLadder;

    // 어빌리티 커밋 (코스트, 쿨다운)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    // 사다리 등반 시작
    CachedMovementComponent->BeginClimbLadder(CurrentLadder, LadderBottom, LadderTop, LadderFacing, LadderStandOff,InitialRailParameter,  true);

    // 이탈 감시 태스크 생성 및 시작
    constexpr float CheckRate = 0.02f; // 50Hz
    ExitMonitorTask = UCYAbilityTask_WaitForLadderExit::CreateWaitForLadderExitTask(this, CurrentLadder, LadderBottom, LadderTop, LadderFacing, CheckRate);
    
    if (ExitMonitorTask)
    {
        ExitMonitorTask->OnExitTop.AddDynamic(this, &ThisClass::HandleLadderExitFromTop);
        ExitMonitorTask->OnExitBottom.AddDynamic(this, &ThisClass::HandleLadderExitFromBottom);
        ExitMonitorTask->OnCancelled.AddDynamic(this, &ThisClass::HandleLadderClimbingCancelled);
        ExitMonitorTask->ReadyForActivation();
    }
    else
    {
        UE_LOG(LogCY, Error, TEXT("ClimbLadder_Enter: Failed to create exit monitor task"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

bool UCYGameplayAbility_ClimbLadder_Enter::CanExtractLadderInfo(const FGameplayEventData* TriggerEventData, ACYLadderBase*& OutLadder, FVector& OutBottomLocation,FVector& OutTopLocation,FVector& OutFacingDirection, float& OutLadderStandOff, float& OutInitialRailParameter, bool& OutIsClimbingUp) const
{
    // 이벤트 데이터 검증
    if (!TriggerEventData || !TriggerEventData->Target)
    {
        UE_LOG(LogCY, Warning, TEXT("ExtractLadderInfo: No event data or target"));
        return false;
    }

    // 사다리 캐스팅
    OutLadder = Cast<ACYLadderBase>(const_cast<AActor*>(TriggerEventData->Target.Get()));
    if (!OutLadder)
    {
        UE_LOG(LogCY, Warning, TEXT("ExtractLadderInfo: Target is not a ladder"));
        return false;
    }

    // 캐릭터 확인
    ACharacter* Character = Cast<ACharacter>(const_cast<AActor*>(TriggerEventData->Instigator.Get()));
    if (!Character)
    {
        UE_LOG(LogCY, Warning, TEXT("ExtractLadderInfo: No valid character"));
        return false;
    }

    // 사다리 기본 정보
    OutBottomLocation = OutLadder->GetBottomWorldLocation();
    OutTopLocation = OutLadder->GetTopWorldLocation();
    OutFacingDirection = OutLadder->GetHorizontalFacingDirection();
    OutLadderStandOff = OutLadder->GetLadderStandOffDistance();

    // 진입 타입 확인
    ELadderEntryType EntryType = OutLadder->GetPlayerEntryType(Character);
    
    // None이면 자동 그랩으로 간주
    if (EntryType == ELadderEntryType::None)
    {
        UE_LOG(LogCY, Log, TEXT("ExtractLadderInfo: No entry type found, treating as auto-grab (Middle)"));
        EntryType = ELadderEntryType::Middle;
    }

    // 등반 방향 결정 (EventMagnitude 우선, 없으면 사다리에서 결정)
    if (FMath::Abs(TriggerEventData->EventMagnitude) > KINDA_SMALL_NUMBER)
    {
        OutIsClimbingUp = TriggerEventData->EventMagnitude > 0;
    }
    else
    {
        OutLadder->DetermineClimbDirection(EntryType, Character, OutIsClimbingUp);
    }

    // 초기 레일 위치 계산
    OutInitialRailParameter = OutLadder->CalculateInitialRailParameter(EntryType, Character);
    
    return true;
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderExitFromTop()
{
    UE_LOG(LogCY, Log, TEXT("ClimbLadder_Enter: Exiting from top"));
    
    if (CachedMovementComponent)
    {
        CachedMovementComponent->EndClimbLadder(/*bStepOffTop=*/true);
    }
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderExitFromBottom()
{
    UE_LOG(LogCY, Log, TEXT("ClimbLadder_Enter: Exiting from bottom"));
    
    if (CachedMovementComponent)
    {
        CachedMovementComponent->EndClimbLadder(/*bStepOffTop=*/false);
    }
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderClimbingCancelled()
{
    UE_LOG(LogCY, Log, TEXT("ClimbLadder_Enter: Climbing cancelled (jump)"));
    
    if (CachedMovementComponent)
    {
        // 점프 취소는 현재 위치에서 떨어짐
        CachedMovementComponent->EndClimbLadder(/*bStepOffTop=*/false);
    }
    
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCYGameplayAbility_ClimbLadder_Enter::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // 태스크 정리
    if (ExitMonitorTask)
    {
        ExitMonitorTask->EndTask();
        ExitMonitorTask = nullptr;
    }

    // 사다리 참조 정리
    CurrentLadder = nullptr;
    
    // 안전 체크: 아직 사다리 타는 중이면 종료
    if (CachedMovementComponent && CachedMovementComponent->IsClimbingLadder())
    {
        UE_LOG(LogCY, Warning, TEXT("ClimbLadder_Enter: Force ending ladder climb in EndAbility"));
        CachedMovementComponent->EndClimbLadder(false);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}