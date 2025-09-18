// Fill out your copyright notice in the Description page of Project Settings.


#include "CYInGameMode.h"

#include "CYInGameState.h"
#include "CYLogChannels.h"
#include "Player/CYPlayerController.h"
#include "Player/CYPlayerState.h"
#include "Character/CYPawnData.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
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
    
	return (CurrentRatio < IdealCopRatio) ? ECYTeamRole::Cop : ECYTeamRole::Robber;
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
			if (UClass* PawnClass = PawnData->LoadPawnClass())
			{
				return PawnClass;
			}
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ACYInGameMode::OnPawnDataLoaded()
{
	bPawnDataLoaded = true;

	for (const TWeakObjectPtr<APlayerController>& PendingPlayer : PendingPlayers)
	{
		if (APlayerController* PC = PendingPlayer.Get())
		{
			AssignRandomPawnDataToPlayer(PC);

			// 이미 기본 Pawn이 스폰됐을 수 있으니 필요 시 교정
			if (APawn* ExistingDefaultPawn = PC->GetPawn())
			{
				UClass* DesiredPawn = GetDefaultPawnClassForController_Implementation(PC);
				if (DesiredPawn && !ExistingDefaultPawn->IsA(DesiredPawn))
				{
					if (PlayerCanRestart(PC))
					{
						PC->UnPossess();
						ExistingDefaultPawn->Destroy();
						RestartPlayer(PC);
					}
				}
			}
		}
	}
	PendingPlayers.Empty();
}


