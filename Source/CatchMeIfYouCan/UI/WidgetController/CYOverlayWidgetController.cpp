// Fill out your copyright notice in the Description page of Project Settings.


#include "CYOverlayWidgetController.h"

#include "AbilitySystem/Attributes/CYVitalSet.h"

void UCYOverlayWidgetController::BroadcastInitialValues()
{
	const UCYVitalSet* VitalSet = CastChecked<UCYVitalSet>(AttributeSet);

	// 초기 체력 및 최대 체력 값을 UI에 전달
	OnHealthChanged.Broadcast(VitalSet->GetHealth());
	OnMaxHealthChanged.Broadcast(VitalSet->GetMaxHealth());
}

void UCYOverlayWidgetController::BindCallbacksToDependencies()
{
	const UCYVitalSet* VitalSet = CastChecked<UCYVitalSet>(AttributeSet);

	// 체력 Attribute의 값이 변경될 때마다 HealthChanged 함수를 호출하도록 바인딩
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalSet->GetHealthAttribute()).AddUObject(this, &UCYOverlayWidgetController::HandleHealthChanged);

	// 최대 체력 Attribute의 값이 변경될 때마다 MaxHealthChanged 함수를 호출하도록 바인딩
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalSet->GetMaxHealthAttribute()).AddUObject(this, &UCYOverlayWidgetController::HandleMaxHealthChanged);
}

void UCYOverlayWidgetController::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UCYOverlayWidgetController::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}
