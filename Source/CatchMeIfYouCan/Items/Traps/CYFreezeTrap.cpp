#include "Items/Traps/CYFreezeTrap.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"

ACYFreezeTrap::ACYFreezeTrap()
{
	ItemName = FText::FromString("Freeze Trap");
	TrapType = ETrapType::Freeze;
	TriggerRadius = 100.0f;
    
	// 게임 이펙트 추가
	TrapEffects.Add(UGE_ImmobilizeTrap::StaticClass());
}