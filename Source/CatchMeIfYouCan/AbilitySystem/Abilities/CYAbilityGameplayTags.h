#pragma once

#include "NativeGameplayTags.h"

namespace CYGameplayTags
{
	// Ability Common
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Action_Jump);

	// Ability Interact
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Action_AbilityInteract);
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Action_AbilityInteract_Object);
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Action_AbilityInteract_Door);
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Action_AbilityInteract_Active);
	
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Action_Climbing);

}