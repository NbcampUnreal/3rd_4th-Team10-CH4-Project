#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "CYConsumableBase.generated.h"

class UGameplayEffect;
class ACYPlayerCharacter;
class UParticleSystem;
class UNiagaraSystem;
class USoundBase;

UCLASS(Abstract)
class CATCHMEIFYOUCAN_API ACYConsumableBase : public ACYItemBase
{
	GENERATED_BODY()

public:
	ACYConsumableBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	TArray<TSubclassOf<UGameplayEffect>> ConsumableEffects;

	// 아이템 사용
	virtual bool UseItem(ACYPlayerCharacter* Character) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Consumable")
	void OnConsumableUsed(ACYPlayerCharacter* Character);
};