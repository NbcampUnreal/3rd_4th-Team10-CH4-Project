// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CYPlayerController.generated.h"

class UCYPawnData;
class ACYPlayerState;
class UCYAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	ACYPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "CY|PlayerController")
	ACYPlayerState* GetCYPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "CY|PlayerController")
	UCYAbilitySystemComponent* GetCYAbilitySystemComponent() const;

	// PawnData 준비 알림 (PlayerState에서 호출)
	void OnPawnDataReady();

	// 동기화된 서버 시간 요청
	float GetServerTime();
	
	float GetSingleTripTime() const { return SingleTripTime; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ReceivedPlayer() override;
	
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Pawn() override;

	// GameState 설정 감지
	UFUNCTION()
	void OnGameStateSet(AGameStateBase* NewGameState);
    
	// 클라이언트 초기화 관리
	void CheckClientInitialization();
	bool CanInitializeClient() const;
	void InitializeClient();
    
	// HUD 초기화 (PawnData 기반)
	void CreateTeamSpecificHUD(const UCYPawnData* PawnData);

	// 네트워크 시간 동기화
	void CheckNetworkTimerSync();
	
	// 서버측에 실제 인게임 시간 요청
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// 클라이언트에 실제 타임 전달
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

private:
	// 초기화 상태 추적
	bool bClientInitialized = false;

	// 초기화 재시도 타이머
	FTimerHandle InitCheckTimer;
    
	// 캐시된 PawnData (팀 변경 감지용)
	UPROPERTY()
	const UCYPawnData* CachedPawnData = nullptr;

	// 초기화 재시도 횟수
	int32 InitializationRetryCount = 0;
	const int32 MaxRetryCount = 50;

	// 클라이언트 서버간 시간 차이
	float ClientServerDeltaTime = 0.f;

	// 클라이언트 <-> 서버 간의 단일 딜레이 시간
	float SingleTripTime = 0.f;

	FTimerHandle ResyncTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category="CY|TimeSync", meta=(ClampMin="0.1", ClampMax="60.0"))
	float ResyncInterval = 5.f;
};