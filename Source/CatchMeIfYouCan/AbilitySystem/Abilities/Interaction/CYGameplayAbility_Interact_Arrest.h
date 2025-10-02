// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYGameplayAbility_Interact_Object.h"
#include "CYGameplayAbility_Interact_Arrest.generated.h"

/**
 * 스턴된 도둑을 체포하여 감방으로 텔레포트하는 인터랙션 GA
 * - Interact_Object 파이프라인 기반(Initialize + Commit + 유효성 재검증)
 * - 서버 권한에서만 상호작용 결과 처리
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Interact_Arrest : public UCYGameplayAbility_Interact_Object
{
	GENERATED_BODY()

public:
	
	UCYGameplayAbility_Interact_Arrest();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:

	/** 감방 위치 탐색(월드에 Tag=="Jail"인 액터를 우선 검색) */
	bool FindJailTransform(FTransform& OutTransform) const;

	/** 팀/상태/거리 등 서버 측 재검증 */
	bool ValidateArrest(const ACYCharacterBase* InstigatorCop, const ACYCharacterBase* TargetRobber) const;

	/** 실제 체포 처리(태그 갱신+텔레포트+AliveRobberCount 감소) */
	void DoArrest(ACYCharacterBase* InstigatorCop, ACYCharacterBase* TargetRobber);

private:

	UPROPERTY(EditDefaultsOnly, Category = "CY|Arrest")
	TSubclassOf<UGameplayEffect> JailStateGameplayEffectClass;
	
	/** 체포 가능 최대 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "CY|Arrest")
	float MaxArrestDistance = 220.f;

	/** 스턴이 아니면 실패 처리할지 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "CY|Arrest")
	bool bFailIfNotStunned = true;
};
