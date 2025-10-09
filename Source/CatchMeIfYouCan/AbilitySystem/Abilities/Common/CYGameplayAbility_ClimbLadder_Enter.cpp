// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_ClimbLadder_Enter.h"

#include "CYLogChannels.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "Character/CYStatusGameplayTags.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_WaitForLadderExit.h"
#include "Actors/CYLadderBase.h"
#include "Character/Components/CYCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
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

    
    const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    const float CapsuleHalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 88.0f;

    // 하단: 캡슐 반높이 + 안전 마진
    const float BottomThreshold = CapsuleHalfHeight + BottomExitSafetyMargin;

    // 상단: 고정값 또는 캡슐 기반
    const float TopThreshold = TopExitSafetyMargin;

    // 이탈 감시 태스크 생성 및 시작
    constexpr float CheckRate = 0.02f; // 50Hz
    ExitMonitorTask = UCYAbilityTask_WaitForLadderExit::CreateWaitForLadderExitTask(this, CurrentLadder, LadderBottom, LadderTop, LadderFacing, CheckRate, BottomThreshold, TopThreshold);

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
    
    if (!CachedMovementComponent)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }
    // 이탈 모니터링 중지 (몽타주 재생 중 다시 트리거되는 것 방지)
    if (ExitMonitorTask)
    {
        ExitMonitorTask->EndTask();
        ExitMonitorTask = nullptr;
    }
    
    // 루트 모션 활성화 플래그 설정
    bIsPlayingExitMontage = true;

    if (!TopExitMontage)
    {
        UE_LOG(LogCY, Warning, TEXT("ExecuteExitTop: No TopExitMontage assigned - ending climb directly"));
        CachedMovementComponent->EndClimbLadder(true);
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    if (UAbilityTask_PlayMontageAndWait* ExitMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("LadderTopExit"), TopExitMontage,TopExitMontagePlayRate ,TopExitMontageStartSection ,true,1.0f,0.0f,true))
    {
        ExitMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnTopExitMontageCompleted);
        ExitMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnTopExitMontageCompleted);
        ExitMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnTopExitMontageCancelled);
        ExitMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnTopExitMontageCancelled);
        ExitMontageTask->ReadyForActivation();
    }
    else
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
    
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderExitFromBottom()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCYGameplayAbility_ClimbLadder_Enter::HandleLadderClimbingCancelled()
{
    UE_LOG(LogCY, Log, TEXT("ClimbLadder_Enter: Climbing cancelled (jump)"));

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCYGameplayAbility_ClimbLadder_Enter::OnTopExitMontageCompleted()
{
    UE_LOG(LogCY, Log, TEXT("ClimbLadder_Enter: Top exit montage completed"));
    
    bIsPlayingExitMontage = false;

    // 어빌리티 정상 종료
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCYGameplayAbility_ClimbLadder_Enter::OnTopExitMontageCancelled()
{
    UE_LOG(LogCY, Warning, TEXT("ClimbLadder_Enter: Top exit montage cancelled"));
    
    bIsPlayingExitMontage = false;

    // 어빌리티 취소
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
    bIsPlayingExitMontage = false;
    
    // 안전 체크: 아직 사다리 타는 중이면 종료
    if (CachedMovementComponent && CachedMovementComponent->IsClimbingLadder())
    {
        UE_LOG(LogCY, Warning, TEXT("ClimbLadder_Enter: Force ending ladder climb in EndAbility"));
        CachedMovementComponent->EndClimbLadder(false);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}