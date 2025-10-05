// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact_Ladder.h"

#include "AbilitySystemComponent.h"
#include "CYLogChannels.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "Actors/CYLadderBase.h"
#include "GameFramework/Character.h"

UCYGameplayAbility_Interact_Ladder::UCYGameplayAbility_Interact_Ladder()
{
    // 사다리 진입 이벤트 태그 설정
    LadderEnterEventTag = CYGameplayTags::Ability_Action_Climbing;

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCYGameplayAbility_Interact_Ladder::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // 사다리 검증
    if (!TriggerEventData || !TriggerEventData->Target)
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }
    ACYLadderBase* Ladder = Cast<ACYLadderBase>(const_cast<AActor*>(TriggerEventData->Target.Get()));
    if (!Ladder)
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    // 캐릭터 확인
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    // 진입 타입 확인 (상/하단만 상호작용 가능)
    ELadderEntryType EntryType = Ladder->GetPlayerEntryType(Character);
    if (EntryType != ELadderEntryType::Top && EntryType != ELadderEntryType::Bottom)
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    // 어빌리티 커밋
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    // 사다리 진입 어빌리티로 이벤트 전달
    if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        // 이벤트 데이터 복사 및 수정
        FGameplayEventData LadderEventData = *TriggerEventData;
        LadderEventData.EventTag = LadderEnterEventTag;
        
        // 추가 정보를 EventMagnitude로 전달 (옵션)
        // 1.0 = 상향, -1.0 = 하향
        bool bClimbUp = Ladder->DetermineClimbDirection(EntryType, Character);
        LadderEventData.EventMagnitude = bClimbUp ? 1.0f : -1.0f;
        
        // ClimbLadder_Enter 어빌리티 트리거
        int32 TriggeredCount = ASC->HandleGameplayEvent(LadderEventData.EventTag, &LadderEventData);
        
        if (TriggeredCount > 0)
        {
            UE_LOG(LogCY, Log, TEXT("Interact_Ladder: Successfully triggered ladder entry (Type: %s)"),
                   EntryType == ELadderEntryType::Top ? TEXT("Top") : TEXT("Bottom"));
        }
        else
        {
            UE_LOG(LogCY, Warning, TEXT("Interact_Ladder: Failed to trigger ladder entry ability"));
        }
    }

    // 상호작용 즉시 완료
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}