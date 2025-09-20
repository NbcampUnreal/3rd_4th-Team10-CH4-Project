// Fill out your copyright notice in the Description page of Project Settings.


#include "CYInGameMode.h"

#include "CYInGameState.h"
#include "CYLogChannels.h"
#include "EngineUtils.h"
#include "Player/CYPlayerController.h"
#include "Player/CYPlayerState.h"
#include "Character/CYPawnData.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Player/CYPlayerStart.h"
#include "Systems/CYAssetManager.h"

ACYInGameMode::ACYInGameMode(const FObjectInitializer& ObjectInitializer)
{
	DefaultPawnClass = nullptr; // GetDefaultPawnClassForController에서 팀 별 클래스를 지정할 예정
	PlayerControllerClass = ACYPlayerController::StaticClass();
	PlayerStateClass = ACYPlayerState::StaticClass();
	GameStateClass = ACYInGameState::StaticClass();
}

void ACYInGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogCY, Warning, TEXT("========================================"));
	UE_LOG(LogCY, Warning, TEXT("GameMode Started on SERVER"));
	UE_LOG(LogCY, Warning, TEXT("DefaultPawnClass: %s"), *GetNameSafe(DefaultPawnClass));
	UE_LOG(LogCY, Warning, TEXT("========================================"));

	CachePlayerStarts();
	
	UCYAssetManager::Get().LoadAllPawnData(
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnPawnDataLoaded)
	);
}

void ACYInGameMode::PostLogin(APlayerController* NewPlayer)
{
	ConnectedPlayerCount++;
	
	if (!ArePawnDataLoaded())
	{
		// PawnData 로드 전이면 대기열에 추가
		PendingPlayers.Add(NewPlayer);
	}
	else
	{
		// PawnData 로드 완료 후에는 바로 배정
		AssignRandomPawnDataToPlayer(NewPlayer);
	}
	
	Super::PostLogin(NewPlayer);
}

void ACYInGameMode::AssignRandomPawnDataToPlayer(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}
	
	ACYPlayerState* CYPS = NewPlayer->GetPlayerState<ACYPlayerState>();
	if (!CYPS)
	{
		return;
	}
	
	// 팀 비율에 따라 결정
	ECYTeamRole AssignedTeam = DetermineTeamForPlayer();
    
	// 팀별 랜덤 PawnData 선택
	UCYPawnData* SelectedPawnData = nullptr;
	if (AssignedTeam != ECYTeamRole::None)
	{
		SelectedPawnData = UCYAssetManager::Get().GetRandomPawnDataByTeam(AssignedTeam);
	}
    
	// 팀별 PawnData가 없으면 전체에서 선택
	if (!SelectedPawnData)
	{
		SelectedPawnData = UCYAssetManager::Get().GetRandomPawnData();
	}
    
	if (SelectedPawnData)
	{
		CYPS->SetPawnData(SelectedPawnData);
	}
}

void ACYInGameMode::Logout(AController* Exiting)
{
	// TODO : Logout 로직 변경 필요
	// 팀 카운트 업데이트

	ConnectedPlayerCount--;
	
	Super::Logout(Exiting);
}

UClass* ACYInGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const ACYPlayerState* CYPS = InController ? InController->GetPlayerState<ACYPlayerState>() : nullptr)
	{
		if (const UCYPawnData* PawnData = CYPS->GetPawnData())
		{
			if (PawnData->PawnClass)
			{
				return PawnData->PawnClass;
			}
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* ACYInGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (ACYPlayerStart* TeamStart = FindPlayerTeamRoleStart(Cast<APlayerController>(Player)))
	{
		return TeamStart;
	}
	
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void ACYInGameMode::OnPawnDataLoaded()
{
	bPawnDataLoaded = true;

	for (const TWeakObjectPtr<APlayerController>& PendingPlayer : PendingPlayers)
	{
		if (APlayerController* PC = PendingPlayer.Get())
		{
			AssignRandomPawnDataToPlayer(PC);

			if (PlayerCanRestart(PC))
			{
				// 이미 기본 Pawn이 스폰됐을 수 있으니 필요 시 교정
				if (APawn* ExistingDefaultPawn = PC->GetPawn())
				{
					UClass* DesiredPawnClass = GetDefaultPawnClassForController_Implementation(PC);
					if (DesiredPawnClass && !ExistingDefaultPawn->IsA(DesiredPawnClass))
					{
						PC->UnPossess();
						ExistingDefaultPawn->Destroy();
					}
				}
				RestartPlayer(PC);
			}
		}
	}
	PendingPlayers.Empty();
}

ECYTeamRole ACYInGameMode::DetermineTeamForPlayer()
{
	// 현재 팀 인원 계산
	int32 CurrentCops = 0;
	int32 CurrentRobbers = 0;
    
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ACYPlayerState* CYPS = Cast<ACYPlayerState>(PS))
		{
			ECYTeamRole Team = CYPS->GetTeamRole();
			if (Team == ECYTeamRole::Cop) CurrentCops++;
			else if (Team == ECYTeamRole::Robber) CurrentRobbers++;
		}
	}
    
	// 2:4 비율 (경찰:도둑)
	float IdealCopRatio = 0.33f;
	float CurrentRatio = (CurrentCops + CurrentRobbers > 0) ? 
		static_cast<float>(CurrentCops) / static_cast<float>(CurrentCops + CurrentRobbers) : 0.0f;
    
	ECYTeamRole Result = (CurrentRatio < IdealCopRatio) ? ECYTeamRole::Cop : ECYTeamRole::Robber;
    
	UE_LOG(LogCY, Log, TEXT("DetermineTeam: Cops=%d, Robbers=%d, Ratio=%.2f, Result=%s"),
		CurrentCops, CurrentRobbers, CurrentRatio,
		Result == ECYTeamRole::Cop ? TEXT("Cop") : TEXT("Robber"));
    
	return Result;
}

ACYPlayerStart* ACYInGameMode::FindPlayerTeamRoleStart(const APlayerController* NewPlayer) const
{
	if (!NewPlayer)
	{
		return nullptr;
	}

	// 플레이어의 팀 정보 가져오기
	ECYTeamRole PlayerTeam = ECYTeamRole::None;
	if (const ACYPlayerState* CYPS = NewPlayer->GetPlayerState<ACYPlayerState>())
	{
		PlayerTeam = CYPS->GetTeamRole();
	}

	// 팀별 PlayerStart 배열 선택
	const TArray<ACYPlayerStart*>* TeamStarts = nullptr;
    
	switch (PlayerTeam)
	{
	case ECYTeamRole::Cop:
		TeamStarts = &CopPlayerStarts;
		UE_LOG(LogCY, Warning, TEXT("Player %s is a Cop!"), *NewPlayer->GetName());
		break;
        
	case ECYTeamRole::Robber:
		TeamStarts = &RobberPlayerStarts;
		UE_LOG(LogCY, Warning, TEXT("Player %s is a Robber!"), *NewPlayer->GetName());
		break;
        
	case ECYTeamRole::None:
	default:
		UE_LOG(LogCY, Warning, TEXT("Player %s has no team assigned!"), *NewPlayer->GetName());
		return nullptr;
	}

	// 선택된 팀 배열이 비어있는지 확인
	if (!TeamStarts || TeamStarts->Num() == 0)
	{
		UE_LOG(LogCY, Error, TEXT("No PlayerStarts found for Team %s!"), 
			PlayerTeam == ECYTeamRole::Cop ? TEXT("Cop") : TEXT("Robber"));
		return nullptr;
	}

	// 이미 우선순위로 정렬되어 있으므로 순서대로 체크
	for (ACYPlayerStart* Start : *TeamStarts)
	{
		if (Start && !Start->bIsOccupied)
		{
			Start->SetOccupied(true);
			return Start;
		}
	}

	// 모든 스폰 포인트가 Occupied면 첫 번째 스폰 포인트 강제 사용 (폴백)
	if (TeamStarts->Num() > 0)
	{
		ACYPlayerStart* FallbackStart = (*TeamStarts)[0];
		UE_LOG(LogCY, Warning, TEXT("All spawn points for Team %s are occupied! Force spawning at: %s"), 
			PlayerTeam == ECYTeamRole::Cop ? TEXT("Cop") : TEXT("Robber"),
			*FallbackStart->GetName());
		return FallbackStart;
	}

	return nullptr;
}

void ACYInGameMode::CachePlayerStarts()
{
	CopPlayerStarts.Empty();
	RobberPlayerStarts.Empty();
    
	// 모든 CYPlayerStart를 팀별로 분류하여 캐싱
	for (TActorIterator<ACYPlayerStart> It(GetWorld()); It; ++It)
	{
		ACYPlayerStart* PlayerStart = *It;
		if (!PlayerStart || !PlayerStart->IsValidLowLevel())
		{
			continue;
		}

		// 팀별로 분류하여 저장
		switch (PlayerStart->AllowedTeam)
		{
		case ECYTeamRole::Cop:
			CopPlayerStarts.Add(PlayerStart);
			break;
            
		case ECYTeamRole::Robber:
			RobberPlayerStarts.Add(PlayerStart);
			break;
            
		case ECYTeamRole::None:
			UE_LOG(LogCY, Warning, TEXT("CYPlayerStart '%s' has AllowedTeam set to None! Please assign a team."), 
				*PlayerStart->GetName());
			break;
		}
	}

	// 우선순위 정렬 (높은 순)
	auto SortByPriority = [](const ACYPlayerStart& A, const ACYPlayerStart& B)
	{
		return A.Priority > B.Priority;
	};
    
	CopPlayerStarts.Sort(SortByPriority);
	RobberPlayerStarts.Sort(SortByPriority);
}


