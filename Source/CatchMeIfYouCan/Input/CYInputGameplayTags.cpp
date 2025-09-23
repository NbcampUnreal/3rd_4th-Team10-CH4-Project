#include "CYInputGameplayTags.h"

namespace CYGameplayTags
{
	// Native Input Tags
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look");

	// Item Input Tags
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interact, "InputTag.Interact");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack, "InputTag.Attack");

	// Inventory Slot Input Tags (1~9)
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot1, "InputTag.UseSlot1");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot2, "InputTag.UseSlot2");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot3, "InputTag.UseSlot3");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot4, "InputTag.UseSlot4");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot5, "InputTag.UseSlot5");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot6, "InputTag.UseSlot6");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot7, "InputTag.UseSlot7");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot8, "InputTag.UseSlot8");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UseSlot9, "InputTag.UseSlot9");

	// Ability Input Tags
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Jump, "InputTag.Ability.Jump");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_WeaponAttack, "InputTag.Ability.WeaponAttack");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_PlaceTrap, "InputTag.Ability.PlaceTrap");
}