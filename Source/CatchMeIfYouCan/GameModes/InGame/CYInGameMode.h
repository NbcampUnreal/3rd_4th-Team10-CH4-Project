// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYTypes/CYInGameTypes.h"
#include "CYInGameMode.generated.h"

class ACYPlayerStart;
class ACYPlayerState;

/**
 * 인게임 모드 - 팀 배정 및 캐릭터 스폰 관리
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYInGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	ACYInGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 플레이어 입장/퇴장 처리 함수
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// 팀별 폰 클래스 반환
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;

protected:
	
	virtual void BeginPlay() override;

	// 임의의 PawnData 할당 
	UFUNCTION(BlueprintCallable, Category = "CY|PawnData")
	void AssignRandomPawnDataToPlayer(APlayerController* NewPlayer);
	
	UFUNCTION()
	void OnPawnDataLoaded();

	ECYTeamRole DetermineTeamForPlayer();

	ACYPlayerStart* FindPlayerTeamRoleStart(const APlayerController* NewPlayer) const;

	bool ArePawnDataLoaded() const { return bPawnDataLoaded; }

	void CachePlayerStarts();
	
private:
	// TODO : 제거 예정 디버깅용 임시 변수
	int32 ConnectedPlayerCount = 0;
	
	// PawnData 로드가 완료된 후에 스폰을 처리해야 하는 플레이어
	TArray<TWeakObjectPtr<APlayerController>> PendingPlayers;
	
	UPROPERTY()
	TArray<ACYPlayerStart*> CopPlayerStarts;
    
	UPROPERTY()
	TArray<ACYPlayerStart*> RobberPlayerStarts;
	
	// PawnData 로드 완료 여부
	bool bPawnDataLoaded = false;
};
