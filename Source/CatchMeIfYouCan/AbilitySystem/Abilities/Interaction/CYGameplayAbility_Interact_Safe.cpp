// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact_Safe.h"

#include "Actors/CYSafe.h"


UCYGameplayAbility_Interact_Safe::UCYGameplayAbility_Interact_Safe()
{
	// 설정은 부모 클래스와 동일
}

void UCYGameplayAbility_Interact_Safe::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData); 

	if (HasAuthority(&CurrentActivationInfo))
	{
		if (ACYSafe* Safe = Cast<ACYSafe>(InteractableActor))
		{
			// Safe는 OnInteractionSuccess에서 GameState 업데이트
			UE_LOG(LogTemp, Log, TEXT("Safe opened by %s"), *GetAvatarActorFromActorInfo()->GetName());
		}
	}
    
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}