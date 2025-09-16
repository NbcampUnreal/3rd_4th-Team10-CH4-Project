// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility.h"

#include "AbilitySystemComponent.h"

UCYGameplayAbility::UCYGameplayAbility()
{
	ActivationPolicy = ECYAbilityActivationPolicy::OnInputTriggered;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCYGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActivationPolicy == ECYAbilityActivationPolicy::OnSpawn)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}
