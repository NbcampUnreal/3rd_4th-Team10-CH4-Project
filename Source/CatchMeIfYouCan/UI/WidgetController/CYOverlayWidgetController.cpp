// Fill out your copyright notice in the Description page of Project Settings.


#include "CYOverlayWidgetController.h"

#include "AbilitySystem/Attributes/CYVitalSet.h"
#include "GameModes/InGame/CYInGameState.h"

void UCYOverlayWidgetController::BroadcastInitialValues()
{
	const UCYVitalSet* VitalSet = CastChecked<UCYVitalSet>(AttributeSet);
	const ACYInGameState* CYGameState = CastChecked<ACYInGameState>(GameState);

	// 초기 VitalSet 정보를 UI에 전달
	OnHealthChanged.Broadcast(VitalSet->GetHealth());
	OnMaxHealthChanged.Broadcast(VitalSet->GetMaxHealth());

	// 초기 인게임 정보를 UI에 전달
	OnTeamCountInfoChanged.Broadcast(CYGameState->GetCopCount(), CYGameState->GetRobberCount());
	OnAliveRobberCountInfoChanged.Broadcast(CYGameState->GetAliveRobberCount());
}

void UCYOverlayWidgetController::BindCallbacksToDependencies()
{
	const UCYVitalSet* VitalSet = Cast<UCYVitalSet>(AttributeSet);
	const ACYInGameState* CYGameState = Cast<ACYInGameState>(GameState);

	// VitalSet 정보 바인딩
	if (VitalSet && AbilitySystemComponent)
	{
		// 체력 Attribute의 값이 변경될 때마다 HealthChanged 함수를 호출하도록 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalSet->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		});

		// 최대 체력 Attribute의 값이 변경될 때마다 MaxHealthChanged 함수를 호출하도록 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalSet->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
	}
	
	// 인게임 정보 바인딩
	if (CYGameState)
	{
		CYGameState->OnTeamCountChanged.AddLambda(
			[this](int32 Cops, int32 Robbers)
		{
			OnTeamCountInfoChanged.Broadcast(Cops, Robbers);
		});

		CYGameState->OnAliveRobberCountChanged.AddLambda(
			[this](int32 AliveCount)
		{
			OnAliveRobberCountInfoChanged.Broadcast(AliveCount);
		});
	}
}

