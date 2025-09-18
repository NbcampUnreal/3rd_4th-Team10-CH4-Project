// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "CYAssetManager.generated.h"

class UCYPawnData;
enum class ECYTeamRole : uint8;
/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UCYAssetManager& Get();

	// PawnData 모두 비동기 로드
	void LoadAllPawnData(const FStreamableDelegate& LoadFinishedCallback);

	// 로드된 PawnData 목록 반환
	bool GetLoadedPawnData(TArray<UCYPawnData*>& OutPawnData) const;

	// 무작위 PawnData
	UCYPawnData* GetRandomPawnData() const;

	// 팀 필터 무작위 PawnData
	UCYPawnData* GetRandomPawnDataByTeam(ECYTeamRole Team) const;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UCYPawnData>> CachedPawnData;
};
