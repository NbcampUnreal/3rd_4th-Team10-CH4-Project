#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CYItemSpawnData.generated.h"

class ACYItemBase;

// 특정 시간대의 아이템 능력치 오버라이드
USTRUCT(BlueprintType)
struct FItemValueOverride
{
	GENERATED_BODY()
    
	// 이 값이 적용될 남은 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 RemainingTimeThreshold = 200;
    
	// 주 능력치 (HealAmount, DamageAmount, SpeedAmount)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0"))
	float PrimaryValue = 0.0f;
    
	// 지속시간 (Freeze, Slow, Invisibility용, 0이면 무시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0"))
	float Duration = 0.0f;
};

// 스폰 가능한 아이템 정보
USTRUCT(BlueprintType)
struct FItemSpawnEntry
{
	GENERATED_BODY()
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ACYItemBase> ItemClass;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float SpawnWeight = 1.0f;
    
	// 남은 시간에 따른 능력치 설정 (내림차순으로 정렬 권장)
	// 예: {200초, 80힐, 0}, {100초, 100힐, 0}
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FItemValueOverride> TimeBasedValues;
};

UCLASS(BlueprintType)
class CATCHMEIFYOUCAN_API UCYItemSpawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
    
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	TArray<FItemSpawnEntry> SpawnableItems;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	float NormalSpawnCooldown = 30.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	float PostPickupSpawnDelay = 10.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings")
	float ItemCheckRadius = 200.0f;
    
	TSubclassOf<ACYItemBase> SelectRandomItem() const;
	FItemValueOverride GetValuesForItem(TSubclassOf<ACYItemBase> ItemClass, float RemainingTime) const;
};