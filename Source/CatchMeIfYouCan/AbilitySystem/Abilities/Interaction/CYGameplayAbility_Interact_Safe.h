// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYGameplayAbility_Interact_Object.h"
#include "CYGameplayAbility_Interact_Safe.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Interact_Safe : public UCYGameplayAbility_Interact_Object
{
	GENERATED_BODY()
	
public:
	UCYGameplayAbility_Interact_Safe();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
