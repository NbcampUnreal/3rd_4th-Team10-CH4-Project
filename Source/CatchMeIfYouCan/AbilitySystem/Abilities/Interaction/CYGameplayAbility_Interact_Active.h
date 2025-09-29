// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYGameplayAbility_Interact_Info.h"
#include "CYGameplayAbility_Interact_Active.generated.h"

class UInputAction;

/**
 * 홀딩 상호작용을 처리하는 핵심 어빌리티 클래스
 * 플레이어가 상호작용 키를 누르고 있는 동안 지속시간을 관리하고 진행률을 추적
 * 동작 흐름:
 * 1. 게임플레이 이벤트로 활성화
 * 2. 즉시 실행 vs 홀딩 분기 결정
 * 3. 홀딩 상태 설정 및 타이머 시작
 * 4. 취소 조건 모니터링 (입력 해제, 위치 이탈)
 * 5. 지속시간 완료 시 네트워크 동기화
 * 6. 실제 상호작용 실행 또는 취소 처리
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Interact_Active : public UCYGameplayAbility_Interact_Info
{
	GENERATED_BODY()
public:
	UCYGameplayAbility_Interact_Active();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:

	/**
	 * 상호작용 유효성 검사 실패 시 호출되는 콜백
	 * 플레이어가 상호작용 대상에서 너무 멀어지거나 각도가 벗어났을 때 홀딩 취소
	 */
	UFUNCTION()
	void OnInvalidInteraction();

	/**
	 * 상호작용 입력이 해제되었을 때 호출되는 콜백
	 * 플레이어가 상호작용 키를 떼면 홀딩 즉시 취소
	 * @param TimeHeld 키를 누르고 있었던 시간 (초 단위)
	 */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	/**
	 * 홀딩 지속시간이 완료되었을 때 호출되는 콜백
	 * 타이머에 의해 호출되며, 네트워크 동기화를 시작
	 */
	UFUNCTION()
	void OnDurationEnded();

	/**
	 * 네트워크 동기화 완료 시 호출되는 콜백
	 * 서버와 클라이언트 간 타이밍을 맞춘 후 실제 상호작용 실행
	 */
	UFUNCTION()
	void OnNetSync();

	/**
	 * 실제 상호작용을 트리거하는 함수
	 * 홀딩 완료 후 상호작용 대상의 어빌리티를 활성화
	 * @return 상호작용 트리거 성공 여부
	 */
	UFUNCTION()
	bool TriggerInteraction();

protected:

	/**
	 * 홀딩 시작 시 기존 움직임 입력을 플러시하기 위해 사용
	 */
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	TObjectPtr<UInputAction> MoveInputAction;

	/**
	 * 상호작용 허용 각도 (도 단위)
	 * 플레이어가 상호작용 대상을 바라보는 각도가 이 값을 벗어나면 홀딩 취소
	*/
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float AcceptanceAngle = 65.f;

	/**
	 * 상호작용 허용 거리 (cm 단위)
	 * 플레이어와 상호작용 대상 간의 거리가 이 값을 초과하면 홀딩 취소
	 */
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float AcceptanceDistance = 10.f;
};
