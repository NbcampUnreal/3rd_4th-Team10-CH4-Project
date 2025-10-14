// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYGameplayAbility_Interact_Object.h"
#include "CYGameplayAbility_Interact_Ladder.generated.h"

/**
 * 사다리 상호작용 브릿지 어빌리티
 * - 키 상호작용을 사다리 Enter 어빌리티로 연결
 * - 사다리의 상/하단 박스에서만 작동
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Interact_Ladder : public UCYGameplayAbility_Interact_Object
{
	GENERATED_BODY()
	
public:
	UCYGameplayAbility_Interact_Ladder();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 사다리 Enter 어빌리티를 활성화 하기 위한 게임플레이 태그 */
	UPROPERTY(EditDefaultsOnly, Category="CY|Ladder")
	FGameplayTag LadderEnterEventTag;
};
