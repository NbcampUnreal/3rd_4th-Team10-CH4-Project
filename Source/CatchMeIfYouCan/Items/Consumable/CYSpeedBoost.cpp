#include "CYSpeedBoost.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"

ACYSpeedBoost::ACYSpeedBoost()
{
	ItemName = FText::FromString("Speed Boost");
    
	ConsumableEffects.Add(UGE_SpeedBoost::StaticClass());
}