// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYWidgetController.h"
#include "CYWidgetDelegates.h"
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

public:
	// UI가 바인딩할 델리게이트들
	UPROPERTY(BlueprintAssignable, Category="CY|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="CY|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	// 팀 정보 델리게이트
	UPROPERTY(BlueprintAssignable, Category="CY|Team")
	FOnTeamCountInfoChanged OnTeamCountInfoChanged;
    
	UPROPERTY(BlueprintAssignable, Category="CY|Team")
	FOnAliveRobberCountInfoChanged OnAliveRobberCountInfoChanged;
};
