// Fill out your copyright notice in the Description page of Project Settings.


#include "CYAssetManager.h"

#include "CYLogChannels.h"
#include "Character/CYPawnData.h"

UCYAssetManager& UCYAssetManager::Get()
{
	check(GEngine);

	if (UCYAssetManager* Singleton = Cast<UCYAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogCY, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to CYAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<UCYAssetManager>();
}

void UCYAssetManager::LoadAllPawnData(const FStreamableDelegate& LoadFinishedCallback)
{
	const FPrimaryAssetType PrimaryAssetType = UCYPawnData::GetPawnAssetType();

	LoadPrimaryAssetsWithType(
		PrimaryAssetType,
		TArray<FName>{},
		FStreamableDelegate::CreateLambda([this, PrimaryAssetType, LoadFinishedCallback]()
		{
			CachedPawnData.Reset();

			TArray<FPrimaryAssetId> PrimaryAssetIds;
			GetPrimaryAssetIdList(PrimaryAssetType, PrimaryAssetIds);

			for (const FPrimaryAssetId& PrimaryAssetId : PrimaryAssetIds)
			{
				// 로드가 되어 있으면 Obj는 유효함
				if (UObject* Obj = GetPrimaryAssetObject(PrimaryAssetId))
				{
					if (UCYPawnData* Data = Cast<UCYPawnData>(Obj))
					{
						CachedPawnData.Add(Data);
					}
				}
			}
			LoadFinishedCallback.ExecuteIfBound();
		})
	);
}

bool UCYAssetManager::GetLoadedPawnData(TArray<UCYPawnData*>& OutPawnData) const
{
	OutPawnData.Reset();
	for (const TWeakObjectPtr<UCYPawnData>& PawnData : CachedPawnData)
	{
		if (UCYPawnData* Data = PawnData.Get())
		{
			OutPawnData.Add(Data);
		}
	}
	return OutPawnData.Num() > 0;
}

UCYPawnData* UCYAssetManager::GetRandomPawnData() const
{
	TArray<UCYPawnData*> Loaded;
	if (!GetLoadedPawnData(Loaded) || Loaded.Num() == 0) return nullptr;
	return Loaded[FMath::RandRange(0, Loaded.Num()-1)];
}

UCYPawnData* UCYAssetManager::GetRandomPawnDataByTeam(ECYTeamRole Team) const
{
	TArray<UCYPawnData*> LoadedPawnData;
	if (!GetLoadedPawnData(LoadedPawnData) || LoadedPawnData.Num() == 0) return nullptr;

	TArray<UCYPawnData*> Filtered;
	for (UCYPawnData* PawnData : LoadedPawnData)
	{
		if (PawnData && PawnData->TeamRole == Team)
		{
			Filtered.Add(PawnData);
		}
	}
	if (Filtered.Num() == 0) return nullptr;
	return Filtered[FMath::RandRange(0, Filtered.Num()-1)];
}
