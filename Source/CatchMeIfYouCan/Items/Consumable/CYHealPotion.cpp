#include "CYHealPotion.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"

ACYHealPotion::ACYHealPotion()
{
	ItemName = FText::FromString("Heal Potion");
    
	ConsumableEffects.Add(UGE_Heal::StaticClass());
}