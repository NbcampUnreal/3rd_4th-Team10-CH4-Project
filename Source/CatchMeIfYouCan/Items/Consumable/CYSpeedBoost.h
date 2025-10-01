#pragma once

#include "CoreMinimal.h"
#include "CYConsumableBase.h"
#include "CYSpeedBoost.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYSpeedBoost : public ACYConsumableBase
{
	GENERATED_BODY()

public:
	ACYSpeedBoost();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed Boost")
	float SpeedBoostAmount = 400.0f;
};