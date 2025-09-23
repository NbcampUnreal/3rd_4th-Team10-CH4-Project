#include "Items/Traps/CYSlowTrap.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"

ACYSlowTrap::ACYSlowTrap()
{
	ItemName = FText::FromString("Slow Trap");
	TrapType = ETrapType::Slow;
	TriggerRadius = 120.0f;
    
	TrapEffects.Add(UGE_SlowTrap::StaticClass());
}