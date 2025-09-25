// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYTypes/CYInGameTypes.h"
#include "CYInGameMode.generated.h"

class ACYInGameState;
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
	virtual void InitGameState() override;

	// 팀별 폰 클래스 반환
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;

	UFUNCTION(BlueprintPure, Category = "CY|InGameState")
	ACYInGameState* GetCYInGameState() const { return CYGameState; }

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
	// 인원/비율 기반 페이즈 전환 시도
	void TryChangeInGamePhase();

	// 2:1 비율 확인
	bool HasRequiredRatio() const;

	void StartPreparing();        
	void StartMatch();           
	
private:
	// TODO : 제거 예정 디버깅용 임시 변수
	int32 ConnectedPlayerCount = 0;
	
	// PawnData 로드가 완료된 후에 스폰을 처리해야 하는 플레이어
	TArray<TWeakObjectPtr<APlayerController>> PendingPlayers;

	// 팀별 PlayerStart 캐싱
	UPROPERTY()
	TArray<ACYPlayerStart*> CopPlayerStarts;
    
	UPROPERTY()
	TArray<ACYPlayerStart*> RobberPlayerStarts;

	UPROPERTY(EditDefaultsOnly, Category="CY|Team", Meta = (ClampMin="1", ClampMax="6"))
	int32 RequiredCopCount = 1;

	UPROPERTY(EditDefaultsOnly, Category="CY|Team", Meta = (ClampMin="1", ClampMax="6"))
	int32 RequiredRobberCount = 2;
	
	// PawnData 로드 완료 여부
	bool bPawnDataLoaded = false;

	// 게임모드에서 게임 스테이트에 현재 페이즈에 맞는 시간을 전달
	UPROPERTY(EditDefaultsOnly, Category="CY|Phase")
	float PreparingCountdownSeconds = 5.f;  

	UPROPERTY(EditDefaultsOnly, Category="CY|Phase")
	float MatchDurationSeconds = 300.f;      

	// 페이즈 전환 타이머
	FTimerHandle PreparingTimerHandle;

	UPROPERTY()
	TObjectPtr<ACYInGameState> CYGameState;
};
