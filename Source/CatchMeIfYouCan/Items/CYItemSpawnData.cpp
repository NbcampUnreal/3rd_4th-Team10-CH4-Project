#include "CYItemSpawnData.h"
#include "Items/CYItemBase.h"

TSubclassOf<ACYItemBase> UCYItemSpawnData::SelectRandomItem() const
{
	if (SpawnableItems.Num() == 0) return nullptr;
    
	float TotalWeight = 0.0f;
	for (const FItemSpawnEntry& Entry : SpawnableItems)
		TotalWeight += Entry.SpawnWeight;
    
	if (TotalWeight <= 0.0f) return nullptr;
    
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;
    
	for (const FItemSpawnEntry& Entry : SpawnableItems)
	{
		CurrentWeight += Entry.SpawnWeight;
		if (RandomValue <= CurrentWeight)
			return Entry.ItemClass;
	}
    
	return SpawnableItems.Last().ItemClass;
}

FItemValueOverride UCYItemSpawnData::GetValuesForItem(TSubclassOf<ACYItemBase> ItemClass, float RemainingTime) const
{
	FItemValueOverride DefaultValues;
	DefaultValues.PrimaryValue = 0.0f;
	DefaultValues.Duration = 0.0f;
    
	for (const FItemSpawnEntry& Entry : SpawnableItems)
	{
		if (Entry.ItemClass == ItemClass)
		{
			const FItemValueOverride* BestMatch = nullptr;
			int32 BestThreshold = INT_MAX;
            
			for (const FItemValueOverride& Override : Entry.TimeBasedValues)
			{
				UE_LOG(LogTemp, Log, TEXT("Checking - RemainingTime:%.1f, Threshold:%d, Condition:%s"), 
					   RemainingTime, Override.RemainingTimeThreshold,
					   RemainingTime <= Override.RemainingTimeThreshold ? TEXT("MATCH") : TEXT("SKIP"));
                
				if (RemainingTime <= Override.RemainingTimeThreshold && 
					Override.RemainingTimeThreshold < BestThreshold)
				{
					BestThreshold = Override.RemainingTimeThreshold;
					BestMatch = &Override;
				}
			}
            
			if (BestMatch)
			{
				UE_LOG(LogTemp, Warning, TEXT("Selected values - Threshold:%d, Primary:%.1f, Duration:%.1f"), 
					   BestThreshold, BestMatch->PrimaryValue, BestMatch->Duration);
				return *BestMatch;
			}
            
			break;
		}
	}
    
	UE_LOG(LogTemp, Warning, TEXT("No override found, using defaults"));
	return DefaultValues;
}