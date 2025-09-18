// Fill out your copyright notice in the Description page of Project Settings.


#include "CYInGameMode.h"

#include "CYInGameState.h"
#include "CYLogChannels.h"
#include "Player/CYPlayerController.h"
#include "Player/CYPlayerState.h"
#include "Character/CYCopCharacter.h"
#include "Character/CYPawnData.h"
#include "Character/CYRobberCharacter.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Systems/CYAssetManager.h"

ACYInGameMode::ACYInGameMode(const FObjectInitializer& ObjectInitializer)
{
	DefaultPawnClass = nullptr; // GetDefaultPawnClassForController에서 팀 별 클래스를 지정할 예정
	PlayerControllerClass = ACYPlayerController::StaticClass();
	PlayerStateClass = ACYPlayerState::StaticClass();
	GameStateClass = ACYInGameState::StaticClass();

	CopCharacterClass = ACYCopCharacter::StaticClass();
	RobberCharacterClass = ACYRobberCharacter::StaticClass();
}

void ACYInGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UE_LOG(LogCY, Warning, TEXT("========================================"));
		UE_LOG(LogCY, Warning, TEXT("GameMode Started on SERVER"));
		UE_LOG(LogCY, Warning, TEXT("DefaultPawnClass: %s"), *GetNameSafe(DefaultPawnClass));
		UE_LOG(LogCY, Warning, TEXT("========================================"));

		UCYAssetManager::Get().LoadAllPawnData(
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnPawnDataLoaded)
		);
	}
}

void ACYInGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (HasAuthority())
	{
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
	}
	
	Super::PostLogin(NewPlayer);
}

void ACYInGameMode::AssignRandomPawnDataToPlayer(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}
	
	if (ACYPlayerState* CYPS = NewPlayer->GetPlayerState<ACYPlayerState>())
	{
		// 필요 시 GetRandomPawnDataByTeam으로 비율 제어 가능
		if (UCYPawnData* PawnData = UCYAssetManager::Get().GetRandomPawnData())
		{
			CYPS->SetPawnData(PawnData);
			UE_LOG(LogCY, Warning, TEXT("[GameMode] Assigned PawnData %s to %s (Team=%d)"),
				*PawnData->GetName(), *CYPS->GetName(), (int32)PawnData->TeamRole);
		}
	}
}

void ACYInGameMode::Logout(AController* Exiting)
{
	// 팀 카운트 업데이트
	if (ACYPlayerState* PS = Exiting->GetPlayerState<ACYPlayerState>())
	{
		if (PS->GetTeamRole() == ECYTeamRole::Cop)
		{
			CopPlayerCount--;
		}
		else if (PS->GetTeamRole() == ECYTeamRole::Robber)
		{
			RobberPlayerCount--;
		}
	}

	ConnectedPlayerCount--;

	UE_LOG(LogCY, Warning, TEXT("[SERVER] Player Disconnected (Remaining: %d, Cops: %d, Robbers: %d)"),
		ConnectedPlayerCount, CopPlayerCount, RobberPlayerCount);

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
