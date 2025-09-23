// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYWidgetController.h"
#include "CYOverlayWidgetController.generated.h"

struct FOnAttributeChangeData;

// Attribute 변경을 브로드캐스트하기 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewHealth);

/**
 * 메인 HUD UI를 관리하는 위젯 컨트롤러
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYOverlayWidgetController : public UCYWidgetController
{
	GENERATED_BODY()

public:
	// 초기값 브로드캐스트
	virtual void BroadcastInitialValues() override;

	// 콜백 함수 바인딩
	virtual void BindCallbacksToDependencies() override;

protected:
	// Attribute 변경 시 호출될 콜백 함수
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

public:
	// UI가 바인딩할 델리게이트들
	UPROPERTY(BlueprintAssignable, Category="CY|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="CY|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;


};
