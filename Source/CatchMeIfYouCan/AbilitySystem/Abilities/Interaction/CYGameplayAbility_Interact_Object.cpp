// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact_Object.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_WaitForInvalidInteraction.h"
#include "Character/CYStatusGameplayTags.h"
#include "Interaction/CYWorldInteractable.h"

UCYGameplayAbility_Interact_Object::UCYGameplayAbility_Interact_Object()
{
    ActivationPolicy = ECYAbilityActivationPolicy::Manual;
	
	AbilityTags.AddTag(CYGameplayTags::Ability_Action_AbilityInteract_Object);
	ActivationOwnedTags.AddTag(CYGameplayTags::Status_Action_AbilityInteract);
}

void UCYGameplayAbility_Interact_Object::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 상호작용 대상 초기화 
	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
	if (InitializeAbility(TargetActor) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 초기화 상태 리셋
	bInitialized = false;

	FCYInteractionQuery Query;
	Query.RequestingAvatar = GetAvatarActorFromActorInfo();
	Query.RequestingController = GetControllerFromActorInfo();

	// 상호작용 가능 여부 확인
	if (Interactable->CanInteraction(Query) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 초기화 완료 표시
	bInitialized = true;

	if (ACYWorldInteractable* WorldInteractable = Cast<ACYWorldInteractable>(TargetActor))
	{
		WorldInteractable->OnInteractionSuccess(GetAvatarActorFromActorInfo());
	}

	// 상호작용 중 플레이어가 범위를 벗어나면 취소
	if (UCYAbilityTask_WaitForInvalidInteraction* InvalidInteractionTask = UCYAbilityTask_WaitForInvalidInteraction::WaitForInvalidInteraction(this, AcceptanceAngle, AcceptanceDistance))
	{
		InvalidInteractionTask->OnInvalidInteraction.AddDynamic(this, &ThisClass::OnInvalidInteraction);
		InvalidInteractionTask->ReadyForActivation();
	}

	// 상호작용 종료 애니메이션 몽타주 재생
	if (UAnimMontage* ActiveEndMontage = InteractionInfo.ActiveEndMontage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("InteractMontage"), ActiveEndMontage, 1.f, NAME_None, true, 1.f, 0.f, false))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}
}

void UCYGameplayAbility_Interact_Object::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCYGameplayAbility_Interact_Object::OnInvalidInteraction()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}