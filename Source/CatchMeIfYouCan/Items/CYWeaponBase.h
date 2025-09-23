#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "CYWeaponBase.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYWeaponBase : public ACYItemBase
{
	GENERATED_BODY()

public:
	ACYWeaponBase();

	// 무기 스탯
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float AttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float AttackCooldown = 2.0f;

	// 무기 전용 함수들
	virtual bool UseItem(ACYPlayerCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Equip(ACYPlayerCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Unequip();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsEquipped() const { return OwningCharacter != nullptr; }

protected:
	UPROPERTY()
	ACYPlayerCharacter* OwningCharacter;
};