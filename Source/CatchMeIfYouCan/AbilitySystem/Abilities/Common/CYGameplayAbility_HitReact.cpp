// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_HitReact.h"

#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "Character/CYStatusGameplayTags.h"

UCYGameplayAbility_HitReact::UCYGameplayAbility_HitReact()

{
    ActivationPolicy = ECYAbilityActivationPolicy::Manual;
	bServerRespectsRemoteAbilityCancellation = true;
	bRetriggerInstancedAbility = true;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	AbilityTags.AddTag(CYGameplayTags::Ability_Action_HitReact);
	ActivationOwnedTags.AddTag(CYGameplayTags::Status_HitReact);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CYGameplayTags::GameplayEvent_HitReact;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UCYGameplayAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (UAbilityTask_NetworkSyncPoint* NetSyncTask = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait))
	{
		NetSyncTask->OnSync.AddDynamic(this, &ThisClass::OnNetSync);
		NetSyncTask->ReadyForActivation();
	}
}

void UCYGameplayAbility_HitReact::OnNetSync()
{
	ACYCharacterBase* TargetCharacter = GetCYCharacterFromActorInfo();
	if (TargetCharacter == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	AActor* InstigatorActor = CurrentEventData.ContextHandle.GetInstigator();

	if (HitReactMontage == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}
	
	if (UAbilityTask_PlayMontageAndWait* HitReactMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("HitReactMontage"), HitReactMontage, 1.f, NAME_None, true))
	{
		HitReactMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->ReadyForActivation();
	}
}

void UCYGameplayAbility_HitReact::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}