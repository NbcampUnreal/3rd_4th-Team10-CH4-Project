// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "CYAbilityTask_WaitForInvalidInteraction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvalidInteraction);

/**
 * 홀딩 상호작용 중 플레이어의 위치와 각도를 지속적으로 모니터링하는 어빌리티 태스크
 * 플레이어가 상호작용 대상에서 너무 멀어지거나 바라보는 각도가 벗어나면 홀딩을 취소
 * - 주기적인 거리 및 각도 검사 (0.05초마다)
 * - 2D 평면 기반 각도 계산 (높이 차이 무시)
 * - 상호작용 시작 시점의 플레이어 상태 캐싱
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYAbilityTask_WaitForInvalidInteraction : public UAbilityTask
{
	GENERATED_BODY()

public:
	UCYAbilityTask_WaitForInvalidInteraction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UCYAbilityTask_WaitForInvalidInteraction* WaitForInvalidInteraction(UGameplayAbility* OwningAbility, float AcceptanceAngle, float AcceptanceDistance);

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	void PerformCheck();

	/**
	 * 플레이어와 상호작용 대상 간의 2D 각도를 계산
	 * 높이 차이는 무시하고 수평면에서의 각도만 계산
	 * @return 계산된 각도 (도 단위)
	 */
	float CalculateAngle2D() const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnInvalidInteraction OnInvalidInteraction;

private:
	float AcceptanceAngle = 0.f;
	float AcceptanceDistance = 0.f;
	
private:
	FTimerHandle CheckTimerHandle;

	/** 상호작용 시작 시점의 플레이어 전방 벡터 (2D, 높이 무시) */
	FVector CachedCharacterForward2D;

	/** 상호작용 시작 시점의 플레이어 위치 */
	FVector CachedCharacterLocation;
};
