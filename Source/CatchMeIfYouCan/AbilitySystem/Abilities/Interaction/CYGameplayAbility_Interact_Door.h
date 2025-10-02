// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYGameplayAbility_Interact_Object.h"
#include "CYGameplayAbility_Interact_Door.generated.h"

/**
 * 문 상호작용을 위한 전용 어빌리티 클래스
 * 플레이어의 위치에 따라 문을 앞쪽/뒤쪽으로 열거나 닫는 기능을 제공
 * 방향 계산:
 * - 문의 Forward 벡터와 (플레이어 위치 - 문 위치) 벡터의 내적 계산
 * - 내적 < 0 → 뒤쪽에서 접근 (Open_Backward)
 * - 내적 >= 0 → 앞쪽에서 접근 (Open_Forward)
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Interact_Door : public UCYGameplayAbility_Interact_Object
{
	GENERATED_BODY()

public:
	UCYGameplayAbility_Interact_Door();

protected:
	/**
	 * 문 상태 확인, 플레이어 위치 기반 방향 계산, 문 열기/닫기 실행
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
