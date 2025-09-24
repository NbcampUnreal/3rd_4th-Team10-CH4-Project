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

protected:
	virtual void BeginPlay() override;
	
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

private:
	// 초기화 상태 추적
	bool bClientInitialized = false;

	// 초기화 재시도 타이머
	FTimerHandle InitCheckTimer;
    
	// 캐시된 PawnData (팀 변경 감지용)
	UPROPERTY()
	const UCYPawnData* CachedPawnData = nullptr;
	
	int32 InitializationRetryCount = 0;
	const int32 MaxRetryCount = 50;

};