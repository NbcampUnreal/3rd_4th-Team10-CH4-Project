// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYInGameMode.generated.h"

/**
 * 
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

protected:
	virtual void BeginPlay() override;

private:
	// 디버깅용 임시 변수
	int32 ConnectedPlayerCount = 0;
	
};
