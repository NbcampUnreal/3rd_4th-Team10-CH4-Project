// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYWidgetController.h"
#include "CYTypes/CYInGameTypes.h"
#include "UI/WidgetController/CYWidgetDelegates.h"
#include "CYOverlayWidgetController.generated.h"

struct FOnAttributeChangeData;

/**
 * 메인 HUD UI를 관리하는 위젯 컨트롤러
 */
UCLASS(BlueprintType, Blueprintable)
class CATCHMEIFYOUCAN_API UCYOverlayWidgetController : public UCYWidgetController
{
	GENERATED_BODY()

public:
	// 초기값 브로드캐스트
	virtual void BroadcastInitialValues() override;

	// 콜백 함수 바인딩
	virtual void BindCallbacksToDependencies() override;

	EGamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }

private:
	// 인게임 페이즈 별 타이머를 통해 UI 업데이트 (PC에서 계산한 Local Predicted값 사용)
	void HandleGamePhaseChanged(EGamePhase NewPhase);
	void StartCountdownTick(EGamePhase NewPhase);
	void StopCountdownTick();
	void TickCountdown();          
	
public:
	// UI가 바인딩할 델리게이트들
	UPROPERTY(BlueprintAssignable, Category="CY|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="CY|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	// 팀 정보 델리게이트
	UPROPERTY(BlueprintAssignable, Category="CY|Role")
	FOnTeamRoleChanged OnTeamRoleChanged;
	
	UPROPERTY(BlueprintAssignable, Category="CY|Team")
	FOnTeamCountInfoChanged OnTeamCountInfoChanged;
    
	UPROPERTY(BlueprintAssignable, Category="CY|Team")
	FOnAliveRobberCountInfoChanged OnAliveRobberCountInfoChanged;

	UPROPERTY(BlueprintAssignable, Category="CY|Phase")
	FOnGamePhaseChangedSignature OnGamePhaseChanged;
	
	// 인게임 시간 델리게이트
	UPROPERTY(BlueprintAssignable, Category="CY|Phase")
	FOnTimeChanged OnPreparingTimeChanged;

	UPROPERTY(BlueprintAssignable, Category="CY|Phase")
	FOnTimeChanged OnMatchTimeChanged;

	// 인게임 타이머 갱신 주기 조절
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CY|Phase", meta=(ClampMin="0.02", ClampMax="1.0"))
	float CountdownTickInterval = 0.1f;

private:
	FTimerHandle CountdownTickHandle;

	// 현재 표시 중인 페이즈
	EGamePhase CurrentGamePhase = EGamePhase::WaitingToStart;
};