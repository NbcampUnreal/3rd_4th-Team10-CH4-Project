// Fill out your copyright notice in the Description page of Project Settings.


#include "CYInGameState.h"

#include "CYLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "Player/CYPlayerController.h"

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
	
	DOREPLIFETIME(ThisClass, PreparingEndServerTimeSeconds);
	DOREPLIFETIME(ThisClass, MatchEndServerTimeSeconds);
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

float ACYInGameState::GetPreparingRemainingTimeLocal() const
{
	const float CurrentLocalPredictedTime = GetSynchronizedServerTimeFromPC();
	return FMath::Max(0.f, PreparingEndServerTimeSeconds - CurrentLocalPredictedTime);
}

float ACYInGameState::GetMatchRemainingTimeLocal() const
{
	const float CurrentLocalPredictedTime = GetSynchronizedServerTimeFromPC();
	return FMath::Max(0.f, MatchEndServerTimeSeconds - CurrentLocalPredictedTime);
}

void ACYInGameState::SetGamePhase_Server(EGamePhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}
	if (CurrentGamePhase == NewPhase)
	{
		return;
	}
	
	CurrentGamePhase = NewPhase;
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
}

void ACYInGameState::StartPreparing_Server(float InCountdownSeconds)
{
	if (!HasAuthority())
	{
		return;
	}
	
	const float StartPreparingTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	PreparingEndServerTimeSeconds = StartPreparingTime + InCountdownSeconds; 
	
	SetGamePhase_Server(EGamePhase::Preparing);
}

void ACYInGameState::StartMatch_Server(float InMatchDurationSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	const float StartMatchTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	MatchEndServerTimeSeconds = StartMatchTime + InMatchDurationSeconds;
	
	SetGamePhase_Server(EGamePhase::InProgress);
}

float ACYInGameState::GetSynchronizedServerTimeFromPC() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0.f;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (ACYPlayerController* CYPC = Cast<ACYPlayerController>(PC))
		{
			return CYPC->GetServerTime();
		}
	}

	// 아직 클라에서 PC가 생성되지 않은 경우 기본적인 로컬 시간 반환
	return World->GetTimeSeconds();
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

void ACYInGameState::OnRep_PreparingEndServerTimeSeconds()
{
	// 서버에서 가끔씩 페이즈별 종료 타이머를 복제
	// OverlayWidgetController(로컬 UI)에서 타이머 로직을 수행해 서버 부담을 줄이는 구조로 만들어 봄

	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
}

void ACYInGameState::OnRep_MatchEndServerTimeSeconds()
{
	// 서버에서 가끔씩 페이즈별 종료 타이머를 복제
	// OverlayWidgetController(로컬 UI)에서 타이머 로직을 수행해 서버 부담을 줄이는 구조로 만들어 봄

	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
}
