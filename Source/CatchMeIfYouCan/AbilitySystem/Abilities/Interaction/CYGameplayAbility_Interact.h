// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "Interaction/CYInteractionInfo.h"
#include "CYGameplayAbility_Interact.generated.h"

/**
 * 기본 상호작용 어빌리티 클래스
 * - 주변 상호작용 가능한 객체 감지 (구체 범위 스캔)
 * - 시선 방향 정밀 타겟팅 (레이캐스트)
 * - 상호작용 입력 대기 및 처리
 * - 홀딩 상호작용 어빌리티 트리거
 * - UI 업데이트 메시지 전송
 * 동작 흐름:
 * 1. 활성화 시 주변 객체 스캔 및 시선 타겟팅 시작
 * 2. 상호작용 입력 대기
 * 3. 입력 감지 시 홀딩 어빌리티 트리거
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Interact : public UCYGameplayAbility
{
	GENERATED_BODY()

public:
	UCYGameplayAbility_Interact();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/**
	 * 감지된 상호작용 정보들을 업데이트하고 UI에 알림
	 * 레이캐스트와 구체 스캔 결과를 받아서 현재 상호작용 정보를 갱신
	 * @param InteractionInfos 새로 감지된 상호작용 정보 배열
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateInteractions(const TArray<FCYInteractionInfo>& InteractionInfos);

	/**
	 * 상호작용을 실제로 트리거하는 함수
	 * 현재 타겟팅된 상호작용에 대해 홀딩 어빌리티를 활성화
	 */
	UFUNCTION(BlueprintCallable)
	void TriggerInteraction();

private:
	/** 상호작용 입력 대기를 시작하는 내부 함수 */
	void WaitInputStart();

	/** 상호작용 입력이 감지되었을 때 호출되는 콜백 함수 */
	UFUNCTION()
	void OnInputStart();

protected:
	/** 현재 감지된 상호작용 정보들 (UI 업데이트 및 트리거에 사용) */
	UPROPERTY(BlueprintReadWrite)
	TArray<FCYInteractionInfo> CurrentInteractionInfos;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float InteractionTraceRange = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float InteractionTraceRate = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float InteractionScanRange = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float InteractionScanRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	bool bShowTraceDebug = false;

	/**
	 * 기본 상호작용 UI 위젯 클래스
	 * 상호작용 정보에 커스텀 위젯이 지정되지 않은 경우 사용
	 */
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;
};
