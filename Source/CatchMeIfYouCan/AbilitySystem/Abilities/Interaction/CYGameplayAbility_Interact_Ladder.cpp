// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact_Ladder.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "Actors/CYLadderBase.h"
#include "GameFramework/Character.h"

UCYGameplayAbility_Interact_Ladder::UCYGameplayAbility_Interact_Ladder()
{
    // 사다리 진입 이벤트 태그 설정
    LadderEnterEventTag = CYGameplayTags::Ability_Action_Climbing;
}

void UCYGameplayAbility_Interact_Ladder::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
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

    // 등반 방향 결정
    bool bClimbUp = false;
    Ladder->DetermineClimbDirection(EntryType, Character, bClimbUp);

    // 사다리 진입 어빌리티로 이벤트 전달
    if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        FGameplayEventData LadderEventData = *TriggerEventData;
        LadderEventData.EventTag = LadderEnterEventTag;
        LadderEventData.EventMagnitude = bClimbUp ? 1.0f : -1.0f;
  
        SendGameplayEvent(LadderEnterEventTag, LadderEventData);
    }

    // 상호작용 즉시 완료
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}