#include "Items/CYItemSpawnData.h"
#include "Items/CYItemBase.h"

FItemSpec UCYItemSpawnData::SelectRandomItemForThreshold(int32 Threshold) const
{
	for (const FTimeThresholdGroup& Group : TimeThresholds)
	{
		if (Group.ThresholdSeconds == Threshold)
		{
			if (Group.AvailableItems.Num() == 0)
			{
				return FItemSpec();
			}
            
			float TotalWeight = 0.0f;
			for (const FItemSpec& Spec : Group.AvailableItems)
				TotalWeight += Spec.SpawnWeight;
            
			if (TotalWeight <= 0.0f)
			{
				return Group.AvailableItems[0];
			}
            
			float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
			float CurrentWeight = 0.0f;
            
			for (const FItemSpec& Spec : Group.AvailableItems)
			{
				CurrentWeight += Spec.SpawnWeight;
				if (RandomValue <= CurrentWeight)
					return Spec;
			}
            
			return Group.AvailableItems.Last();
		}
	}
    
	return FItemSpec();
}

TArray<int32> UCYItemSpawnData::GetAllThresholds() const
{
	TArray<int32> Result;
	for (const FTimeThresholdGroup& Group : TimeThresholds)
	{
		Result.Add(Group.ThresholdSeconds);
	}
	return Result;
}