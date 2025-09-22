#include "CYCombatGameplayTags.h"

namespace CYGameplayTags
{
	// Combat Ability Tags
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_WeaponAttack, "Ability.Combat.WeaponAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_PlaceTrap, "Ability.Combat.PlaceTrap");

	// Team Tags
	UE_DEFINE_GAMEPLAY_TAG(Team_Cop, "Team.Cop");
	UE_DEFINE_GAMEPLAY_TAG(Team_Robber, "Team.Robber");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Attacking, "State.Combat.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Stunned, "State.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Captured, "State.Captured");
	UE_DEFINE_GAMEPLAY_TAG(State_Jail, "State.Jail");
	UE_DEFINE_GAMEPLAY_TAG(State_Free, "State.Free");        

	// Cooldown Tags
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Combat_WeaponAttack, "Cooldown.Combat.WeaponAttack");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Combat_TrapPlace, "Cooldown.Combat.TrapPlace");

	// Effect Tags
	UE_DEFINE_GAMEPLAY_TAG(Effect_Debuff_Slow, "Effect.Debuff.Slow");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Debuff_Freeze, "Effect.Debuff.Freeze");
}