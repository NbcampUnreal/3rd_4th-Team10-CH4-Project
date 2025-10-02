#include "CYInvisibilityPotion.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"

ACYInvisibilityPotion::ACYInvisibilityPotion()
{
	ItemName = FText::FromString("Invisibility Potion");
    
	ConsumableEffects.Add(UGE_Invisibility::StaticClass());
}