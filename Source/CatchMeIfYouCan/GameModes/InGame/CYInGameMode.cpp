// Fill out your copyright notice in the Description page of Project Settings.


#include "CYInGameMode.h"

#include "CYInGameState.h"
#include "CYLogChannels.h"
#include "Player/CYPlayerController.h"
#include "Player/CYPlayerState.h"

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

	if (HasAuthority())
	{
		UE_LOG(LogCY, Warning, TEXT("========================================"));
		UE_LOG(LogCY, Warning, TEXT("GameMode Started on SERVER"));
		UE_LOG(LogCY, Warning, TEXT("DefaultPawnClass: %s"), *GetNameSafe(DefaultPawnClass));
		UE_LOG(LogCY, Warning, TEXT("========================================"));
	}
}

void ACYInGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ConnectedPlayerCount++;

	if (ACYPlayerState* PS = NewPlayer->GetPlayerState<ACYPlayerState>())
	{
		// 간단한 플레이어 이름 설정
		FString PlayerName = FString::Printf(TEXT("Player_%d"), ConnectedPlayerCount);
		PS->SetPlayerName(PlayerName);
        
		UE_LOG(LogCY, Warning, TEXT("[SERVER] Player Connected: %s (Total: %d)"), 
			*PlayerName, ConnectedPlayerCount);
	}
    
	// 스폰 확인 로그
	if (APawn* PlayerPawn = NewPlayer->GetPawn())
	{
		UE_LOG(LogCY, Display, TEXT("  - Spawned Pawn: %s at %s"), 
			*PlayerPawn->GetName(), 
			*PlayerPawn->GetActorLocation().ToString());
	}
}

void ACYInGameMode::Logout(AController* Exiting)
{
	ConnectedPlayerCount--;
    
	UE_LOG(LogCY, Warning, TEXT("[SERVER] Player Disconnected (Remaining: %d)"), ConnectedPlayerCount);
	
	Super::Logout(Exiting);
}
