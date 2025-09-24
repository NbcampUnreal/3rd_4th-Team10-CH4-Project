// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYTypes/CYInGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "CYInGameState.generated.h"


// 델리게이트 (UI 업데이트용)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeamCountChanged, int32 /*CopCount*/, int32 /*RobberCount*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAliveRobberCountChanged, int32 /*AliveRobberCount*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase /*NewPhase*/);

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYInGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	
    ACYInGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 팀 인원 업데이트 함수 (서버 전용 함수)
	UFUNCTION(BlueprintCallable, Category = "CY|Teams")
	void UpdateTeamCount(ECYTeamRole TeamRole, int32 Delta);

	// 살아있는 도둑 수 업데이트 함수 (서버 전용 함수)
	UFUNCTION(BlueprintCallable, Category = "CY|Teams")
	void UpdateAliveRobberCount(int32 NewCount);

	// 팀 비율 계산 헬퍼
	UFUNCTION(BlueprintPure, Category = "CY|Teams")
	float GetCopRatio() const;

	UFUNCTION(BlueprintPure, Category= "CY|Teams")
	int32 GetCopCount() const { return CopCount; }

	UFUNCTION(BlueprintPure, Category= "CY|Teams")
	int32 GetRobberCount() const { return RobberCount; }

	UFUNCTION(BlueprintPure, Category= "CY|Teams")
	int32 GetAliveRobberCount() const { return AliveRobberCount; }
	
protected:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_TeamCounts();

	UFUNCTION()
	void OnRep_AliveRobberCount();

	UFUNCTION()
	void OnRep_GamePhase();

	UFUNCTION()
	void OnRep_RemainingTime();

public:
	mutable FOnTeamCountChanged OnTeamCountChanged;
	mutable FOnAliveRobberCountChanged OnAliveRobberCountChanged;
	mutable FOnGamePhaseChanged OnGamePhaseChanged;

private:
    // 팀별 인원수 (네트워크 복제)
    UPROPERTY(ReplicatedUsing = OnRep_TeamCounts)
    int32 CopCount = 0;

    UPROPERTY(ReplicatedUsing = OnRep_TeamCounts)
    int32 RobberCount = 0;

    // 남은 자유로운 도둑 수 (체포되지 않은)
    UPROPERTY(ReplicatedUsing = OnRep_AliveRobberCount)
    int32 AliveRobberCount = 0;
	
    // 게임 상태
    UPROPERTY(ReplicatedUsing = OnRep_GamePhase)
    EGamePhase CurrentGamePhase = EGamePhase::WaitingToStart;

    // 남은 시간
    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
    float RemainingTime = 300.0f; // 5분
};
