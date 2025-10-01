#include "CYConsumableBase.h"
#include "Character/CYPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "CYHealPotion.h"
#include "CYInvisibilityPotion.h"
#include "CYSpeedBoost.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"
#include "Kismet/GameplayStatics.h"

ACYConsumableBase::ACYConsumableBase()
{
    ItemName = FText::FromString("Base Consumable");
    ItemType = EItemType::Consumable;
    MaxStackCount = 10; // 소비 아이템은 스택 가능
}

bool ACYConsumableBase::UseItem(ACYPlayerCharacter* Character)
{
    if (!Character || !HasAuthority()) return false;

    UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
    if (!ASC) return false;

    bool bSuccess = false;
    
	// 모든 아이템이 GE를 사용
	for (TSubclassOf<UGameplayEffect> EffectClass : ConsumableEffects)
	{
		if (!EffectClass) continue;

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
		if (EffectSpec.IsValid())
		{
			// Heal은 Caller로 힐량 처리
			if (EffectClass->IsChildOf(UGE_Heal::StaticClass()))
			{
				if (ACYHealPotion* HealPotion = Cast<ACYHealPotion>(this))
				{
					EffectSpec.Data->SetSetByCallerMagnitude(FName("HealAmount"), HealPotion->HealAmount);
				}
			}
			// Caller로 속도 증가량 처리
			else if (EffectClass->IsChildOf(UGE_SpeedBoost::StaticClass()))
			{
				if (ACYSpeedBoost* SpeedBoost = Cast<ACYSpeedBoost>(this))
				{
					EffectSpec.Data->SetSetByCallerMagnitude(FName("SpeedBoostAmount"), SpeedBoost->SpeedBoostAmount);
				}
			}

			ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
			bSuccess = true;
		}
	}

	if (bSuccess)
	{
		OnConsumableUsed(Character);
		UE_LOG(LogTemp, Warning, TEXT("%s used consumable: %s"), 
			   *Character->GetName(), *ItemName.ToString());
	}
	
    return bSuccess;
}