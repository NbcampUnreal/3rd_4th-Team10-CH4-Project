// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact_Info.h"

UCYGameplayAbility_Interact_Info::UCYGameplayAbility_Interact_Info()
{
    
}

bool UCYGameplayAbility_Interact_Info::InitializeAbility(AActor* TargetActor)
{
	TScriptInterface<ICYInteractable> TargetInteractable(TargetActor);
	if (TargetInteractable)
	{
		FCYInteractionQuery InteractionQuery;
		InteractionQuery.RequestingAvatar = GetAvatarActorFromActorInfo();
		InteractionQuery.RequestingController = GetControllerFromActorInfo();

		Interactable = TargetInteractable;
		InteractableActor = TargetActor;

		TArray<FCYInteractionInfo> InteractionInfos;
		FCYInteractionInfoBuilder InteractionInfoBuilder(Interactable, InteractionInfos);
		Interactable->GatherPostInteractionInfos(InteractionQuery, InteractionInfoBuilder);
		InteractionInfo = InteractionInfos[0];

		return true;
	}

	return false;
}
