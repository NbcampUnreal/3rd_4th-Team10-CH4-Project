// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/CYCharacterBase.h"
#include "Input/CYEnhancedPlayerInput.h"

UCYGameplayAbility::UCYGameplayAbility()
{
	ActivationPolicy = ECYAbilityActivationPolicy::OnInputTriggered;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCYGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActivationPolicy == ECYAbilityActivationPolicy::OnSpawn)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}

ACYCharacterBase* UCYGameplayAbility::GetCYCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ACYCharacterBase>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

AController* UCYGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

void UCYGameplayAbility::FlushPressedInput(UInputAction* InputAction)
{
	if (CurrentActorInfo)
	{
		if (APlayerController* PlayerController = CurrentActorInfo->PlayerController.Get())
		{
			if (UCYEnhancedPlayerInput* PlayerInput = Cast<UCYEnhancedPlayerInput>(PlayerController->PlayerInput))
			{
				PlayerInput->FlushPressedInput(InputAction);
			}
		}
	}
}
