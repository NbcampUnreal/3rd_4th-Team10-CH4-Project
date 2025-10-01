#pragma once

#include "CoreMinimal.h"
#include "CYConsumableBase.h"
#include "CYHealPotion.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYHealPotion : public ACYConsumableBase
{
	GENERATED_BODY()

public:
	ACYHealPotion();

	// 회복량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heal Potion")
	float HealAmount = 40.0f;
};