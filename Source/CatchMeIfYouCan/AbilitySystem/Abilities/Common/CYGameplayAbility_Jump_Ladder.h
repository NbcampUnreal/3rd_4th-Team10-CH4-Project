// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "CYGameplayAbility_Jump_Ladder.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Jump_Ladder : public UCYGameplayAbility
{
	GENERATED_BODY()

public:
	UCYGameplayAbility_Jump_Ladder();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 사다리에서 점프 시 수평 방향 강도 */
	UPROPERTY(EditDefaultsOnly, Category="CY|Jump", meta=(ClampMin="0.0", ClampMax="2000.0"))
	float LadderBackwardJumpForce = 400.f;
	
	/** 사다리에서 점프 시 수직 방향 강도 */
	UPROPERTY(EditDefaultsOnly, Category="CY|Jump", meta=(ClampMin="0.0", ClampMax="2000.0"))
	float LadderUpwardJumpForce = 500.f;

	/** 카메라 회전 속도 (0=즉시, 값이 클수록 느림) */
	UPROPERTY(EditDefaultsOnly, Category="CY|Jump|Camera", meta=(ClampMin="0.0", ClampMax="20.0"))
	float CameraRotationInterpSpeed = 0.0f; // 0 = 즉시 회전

	/** 카메라도 함께 회전시킬지 여부 */
	UPROPERTY(EditDefaultsOnly, Category="CY|Jump|Camera")
	bool bRotateCamera = true;
};
