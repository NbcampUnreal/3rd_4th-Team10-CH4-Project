// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CYGameplayAbility.generated.h"

class ACYCharacterBase;

UENUM(BlueprintType)
enum class ECYAbilityActivationPolicy : uint8
{
	OnInputTriggered, //Input이 Trigger 되었을 경우 (Pressed/Released) 
	WhileInputActive, // Input이 Held되어 있을 경우 
	OnSpawn, // avatar가 생성되었을 경우 바로 할당(패시브 스킬 등) 
};

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UCYGameplayAbility();

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	ACYCharacterBase* GetCYCharacterFromActorInfo() const;
	ECYAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CY|AbilityActivation")
	ECYAbilityActivationPolicy ActivationPolicy;
};
