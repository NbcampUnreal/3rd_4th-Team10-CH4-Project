// Fill out your copyright notice in the Description page of Project Settings.


#include "CYOverlayWidgetController.h"

#include "CYLogChannels.h"
#include "AbilitySystem/Attributes/CYVitalSet.h"
#include "GameModes/InGame/CYInGameState.h"
#include "Player/CYPlayerState.h"

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

	CurrentGamePhase = CYGameState->GetCurrentGamePhase();
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
	OnSafeCountInfoChanged.Broadcast(CYGameState->GetOpenedSafeCount(), CYGameState->GetTotalSafeCount());
	
	switch (CurrentGamePhase)
	{
	case EGamePhase::Preparing:
		OnPreparingTimeChanged.Broadcast(CYGameState->GetPreparingRemainingTimeLocal());
		StartCountdownTick(CurrentGamePhase);
		break;
	case EGamePhase::InProgress:
		OnMatchTimeChanged.Broadcast(CYGameState->GetMatchRemainingTimeLocal());
		StartCountdownTick(CurrentGamePhase);
		break;
	default:
		break;
	}
}

void UCYOverlayWidgetController::BindCallbacksToDependencies()
{
	const UCYVitalSet* VitalSet = Cast<UCYVitalSet>(AttributeSet);
	const ACYInGameState* CYGameState = Cast<ACYInGameState>(GameState);
	const ACYPlayerState* CYPS = Cast<ACYPlayerState>(PlayerState);

	// if (CYPS)
	// {
	// 	TWeakObjectPtr<UCYOverlayWidgetController> WeakThis(this);
	// 	// PlayerState의 네이티브 멀티캐스트 → Lambda로 수신
	// 	const_cast<ACYPlayerState*>(CYPS)->FOnTeamRoleChanged.AddLambda(
	// 		[WeakThis](ECYTeamRole NewRole)
	// 		{
	// 			if (WeakThis.IsValid())
	// 			{
	// 				WeakThis->OnTeamRoleChanged.Broadcast(NewRole); // BP로 재브로드캐스트
	// 			}
	// 		});
	// }

	TWeakObjectPtr<UCYOverlayWidgetController> WeakThis(this);
	// VitalSet 정보 바인딩
	if (VitalSet && AbilitySystemComponent)
	{
		// 체력 Attribute의 값이 변경될 때마다 HealthChanged 함수를 호출하도록 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalSet->GetHealthAttribute()).AddLambda(
			[WeakThis](const FOnAttributeChangeData& Data)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnHealthChanged.Broadcast(Data.NewValue);
			}
		});

		// 최대 체력 Attribute의 값이 변경될 때마다 MaxHealthChanged 함수를 호출하도록 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalSet->GetMaxHealthAttribute()).AddLambda(
			[WeakThis](const FOnAttributeChangeData& Data)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		});
	}
	
	// 인게임 정보 바인딩
	if (CYGameState)
	{
		CYGameState->OnTeamCountChanged.AddLambda(
			[WeakThis](int32 Cops, int32 Robbers)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnTeamCountInfoChanged.Broadcast(Cops, Robbers);
			}
		});

		CYGameState->OnAliveRobberCountChanged.AddLambda(
			[WeakThis](int32 AliveCount)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnAliveRobberCountInfoChanged.Broadcast(AliveCount);
			}
		});

		CYGameState->OnGamePhaseChanged.AddLambda(
			[WeakThis](EGamePhase NewPhase)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnGamePhaseChanged.Broadcast(NewPhase);
			}
		});

		CYGameState->OnGamePhaseChanged.AddUObject(this, &ThisClass::HandleGamePhaseChanged);

		CYGameState->OnSafeCountChanged.AddLambda(
		   [WeakThis](int32 Opened, int32 Total)
	   {
		   if (WeakThis.IsValid())
		   {
			   WeakThis->OnSafeCountInfoChanged.Broadcast(Opened, Total);
		   }
	   });
	}
}

void UCYOverlayWidgetController::HandleGamePhaseChanged(EGamePhase NewPhase)
{
	CurrentGamePhase = NewPhase;

	switch (NewPhase)
	{
	case EGamePhase::Preparing:
	case EGamePhase::InProgress:
		StartCountdownTick(NewPhase);
		break;
	default:
		StopCountdownTick();
		break;
	}
}

void UCYOverlayWidgetController::StartCountdownTick(EGamePhase NewPhase)
{
	if (GetWorld())
	{
		StopCountdownTick(); // 중복 방지
		GetWorld()->GetTimerManager().SetTimer(
			CountdownTickHandle,
			this,
			&ThisClass::TickCountdown,
			CountdownTickInterval,
			true
		);
	}
}

void UCYOverlayWidgetController::StopCountdownTick()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTickHandle);
	}
}

void UCYOverlayWidgetController::TickCountdown()
{
	const ACYInGameState* CYGS = Cast<ACYInGameState>(GameState);
	if (!CYGS)
	{
		return;
	}
	
	switch (CurrentGamePhase)
	{
	case EGamePhase::Preparing:
		OnPreparingTimeChanged.Broadcast(CYGS->GetPreparingRemainingTimeLocal());
		break;
	case EGamePhase::InProgress:
		OnMatchTimeChanged.Broadcast(CYGS->GetMatchRemainingTimeLocal());
		break;
	default:
		// 다른 페이즈면 틱 멈춤
		StopCountdownTick();
		break;
	}
} 

