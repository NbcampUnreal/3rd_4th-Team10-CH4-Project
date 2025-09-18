// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYTypes/CYTeamType.h"
#include "CYInGameMode.generated.h"

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

	// 임의의 PawnData 할당 
	UFUNCTION(BlueprintCallable, Category = "CY|PawnData")
	void AssignRandomPawnDataToPlayer(APlayerController* NewPlayer);

protected:
	virtual void BeginPlay() override;

	// TODO : 제거 예정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CY|Characters")
	TSubclassOf<APawn> CopCharacterClass;

	// TODO : 제거 예정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CY|Characters")
	TSubclassOf<APawn> RobberCharacterClass;

	UFUNCTION()
	void OnPawnDataLoaded();

	bool ArePawnDataLoaded() const { return bPawnDataLoaded; }

private:
	// TODO : 제거 예정 디버깅용 임시 변수
	int32 ConnectedPlayerCount = 0;

	// 팀별 플레이어 수 추적
	int32 CopPlayerCount = 0;
	int32 RobberPlayerCount = 0;

	
	TArray<TWeakObjectPtr<APlayerController>> PendingPlayers;
	
	bool bPawnDataLoaded = false;
};
