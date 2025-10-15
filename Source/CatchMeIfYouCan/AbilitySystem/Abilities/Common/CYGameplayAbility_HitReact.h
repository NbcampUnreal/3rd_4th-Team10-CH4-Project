// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "CYGameplayAbility_HitReact.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_HitReact : public UCYGameplayAbility
{
	GENERATED_BODY()

public:
	UCYGameplayAbility_HitReact();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UFUNCTION()
	void OnNetSync();
	
	UFUNCTION()
	void OnMontageFinished();

private:
	UPROPERTY(EditDefaultsOnly, Category = "CY|Hit")
	TObjectPtr<UAnimMontage> HitReactMontage;
};
