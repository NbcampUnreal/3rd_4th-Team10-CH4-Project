#pragma once

#include "CoreMinimal.h"
#include "Items/Traps/CYTrapBase.h"
#include "CYDamageTrap.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYDamageTrap : public ACYTrapBase
{
	GENERATED_BODY()

public:
	ACYDamageTrap();
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Trap")
	float DamageAmount = 35.0f;
};