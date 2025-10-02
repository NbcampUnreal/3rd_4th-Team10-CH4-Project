#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CYItemSpawnData.generated.h"

class ACYItemBase;

USTRUCT(BlueprintType)
struct FItemSpec
{
	GENERATED_BODY()
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ACYItemBase> ItemClass;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float SpawnWeight = 1.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0"))
	float PrimaryValue = 0.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0"))
	float Duration = 0.0f;
};

USTRUCT(BlueprintType)
struct FTimeThresholdGroup
{
	GENERATED_BODY()
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 ThresholdSeconds = 200;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FItemSpec> AvailableItems;
};

UCLASS(BlueprintType)
class CATCHMEIFYOUCAN_API UCYItemSpawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
    
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	TArray<FTimeThresholdGroup> TimeThresholds;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	float NormalSpawnCooldown = 30.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	float PostPickupSpawnDelay = 10.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	float ItemCheckRadius = 200.0f;
    
	FItemSpec SelectRandomItemForThreshold(int32 Threshold) const;
	TArray<int32> GetAllThresholds() const;
};