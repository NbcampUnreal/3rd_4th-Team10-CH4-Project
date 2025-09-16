// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Jump.h"

UCYGameplayAbility_Jump::UCYGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ECYAbilityActivationPolicy::OnInputTriggered;
}

