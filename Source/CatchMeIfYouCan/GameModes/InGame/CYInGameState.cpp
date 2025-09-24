// Fill out your copyright notice in the Description page of Project Settings.


#include "CYInGameState.h"

#include "CYLogChannels.h"
#include "Net/UnrealNetwork.h"

ACYInGameState::ACYInGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void ACYInGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CopCount);
	DOREPLIFETIME(ThisClass, RobberCount);
	DOREPLIFETIME(ThisClass, AliveRobberCount);
	DOREPLIFETIME(ThisClass, CurrentGamePhase);
	DOREPLIFETIME(ThisClass, RemainingTime);
}

void ACYInGameState::UpdateTeamCount(ECYTeamRole TeamRole, int32 Delta)
{
	if (!HasAuthority())
	{
		return;
	}
	
	switch (TeamRole)
	{
	case ECYTeamRole::Cop:
		CopCount = FMath::Max(0, CopCount + Delta);
		break;
	case ECYTeamRole::Robber:
		RobberCount = FMath::Max(0, RobberCount + Delta);
		AliveRobberCount = FMath::Clamp(AliveRobberCount + Delta, 0, RobberCount);
		break;
	default:
		break;
	}
    
	// 델리게이트 브로드캐스트 (리슨 서버에서 UI 업데이트)
	OnTeamCountChanged.Broadcast(CopCount, RobberCount);
    
	UE_LOG(LogCY, Warning, TEXT("Team Counts Updated - Cops: %d, Robbers: %d (Alive: %d)"), 
		   CopCount, RobberCount, AliveRobberCount);
}

void ACYInGameState::UpdateAliveRobberCount(int32 NewCount)
{
	if (!HasAuthority())
	{
		return;
	}
	
	AliveRobberCount = FMath::Max(0, NewCount);
	OnAliveRobberCountChanged.Broadcast(AliveRobberCount);
    
	// 승리 조건 체크
	if (CurrentGamePhase == EGamePhase::InProgress && AliveRobberCount == 0)
	{
		// 도둑이 다 잡힌 경우 경찰 승리!
		CurrentGamePhase = EGamePhase::CopsWin;
		OnGamePhaseChanged.Broadcast(CurrentGamePhase);
	}
}

float ACYInGameState::GetCopRatio() const
{
	int32 TotalPlayers = CopCount + RobberCount;
	if (TotalPlayers == 0)
	{
		return 0.0f;
	}
	return static_cast<float>(CopCount) / static_cast<float>(TotalPlayers);
}

void ACYInGameState::OnRep_TeamCounts()
{
	OnTeamCountChanged.Broadcast(CopCount, RobberCount);
}

void ACYInGameState::OnRep_AliveRobberCount()
{
	OnAliveRobberCountChanged.Broadcast(AliveRobberCount);
}

void ACYInGameState::OnRep_GamePhase()
{
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
}

void ACYInGameState::OnRep_RemainingTime()
{
	// UI 업데이트는 위젯에서 직접 처리
}