#include "Items/Traps/CYDamageTrap.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"

ACYDamageTrap::ACYDamageTrap()
{
	ItemName = FText::FromString("Damage Trap");
	TrapType = ETrapType::Damage;
	TriggerRadius = 90.0f;

	// 데미지 트랩 전용 이펙트 추가
	TrapEffects.Add(UGE_DamageTrap::StaticClass());
}