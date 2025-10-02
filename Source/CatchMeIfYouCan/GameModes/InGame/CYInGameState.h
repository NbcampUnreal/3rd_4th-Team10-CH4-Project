// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYTypes/CYInGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "UI/WidgetController/CYWidgetDelegates.h"
#include "CYInGameState.generated.h"


class ACYJailPoint;

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

	UFUNCTION(BlueprintPure, Category= "CY|Phase")
	EGamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }

	// 준비 시간 접근 (로컬에서 사용)
	UFUNCTION(BlueprintPure)
	float GetPreparingRemainingTimeLocal() const;

	// 남은 게임 시간 접근 (로컬에서 사용)
	UFUNCTION(BlueprintPure)
	float GetMatchRemainingTimeLocal() const;

	// 서버 전용 설정 함수들 (GameMode가 호출)
	void SetGamePhase_Server(EGamePhase NewPhase);
	// 현재 경찰과 도둑 2:1 달성 시 호출
	void StartPreparing_Server(float InCountdownSeconds);
	// 대기 종료 후 게임 시작시 호출
	void StartMatch_Server(float InMatchDurationSeconds);

	void InitAliveCountsMatchStart();

	float GetSynchronizedServerTimeFromPC() const;

	void SetJailPoint(ACYJailPoint* InJailPoint);
	ACYJailPoint* GetJailPoint() const { return JailPoint; }
	
protected:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_TeamCounts();

	UFUNCTION()
	void OnRep_AliveRobberCount();

	UFUNCTION()
	void OnRep_GamePhase();

	UFUNCTION()
	void OnRep_PreparingEndServerTimeSeconds();

	UFUNCTION()
	void OnRep_MatchEndServerTimeSeconds();

public:
	// UI 바인딩용 델리게이트(리슨 서버 포함)
	mutable FOnTeamCountChanged OnTeamCountChanged;
	mutable FOnAliveRobberCountChanged OnAliveRobberCountChanged;
	mutable FOnGamePhaseChanged OnGamePhaseChanged;

private:
    // 팀별 인원수
    UPROPERTY(ReplicatedUsing = OnRep_TeamCounts)
    int32 CopCount = 0;

    UPROPERTY(ReplicatedUsing = OnRep_TeamCounts)
    int32 RobberCount = 0;

    // 잡히지 않은 도둑 수
    UPROPERTY(ReplicatedUsing = OnRep_AliveRobberCount)
    int32 AliveRobberCount = 0;
	
    // 게임 상태
    UPROPERTY(ReplicatedUsing = OnRep_GamePhase)
    EGamePhase CurrentGamePhase = EGamePhase::WaitingToStart;

	UPROPERTY(ReplicatedUsing=OnRep_PreparingEndServerTimeSeconds)
	float PreparingEndServerTimeSeconds = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_MatchEndServerTimeSeconds)
	float MatchEndServerTimeSeconds = 0.f;
	
	UPROPERTY()
	TObjectPtr<ACYJailPoint> JailPoint;
};
