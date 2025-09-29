// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "Interaction/CYInteractable.h"
#include "CYGameplayAbility_Interact_Info.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_Interact_Info : public UCYGameplayAbility
{
	GENERATED_BODY()
public:
	UCYGameplayAbility_Interact_Info();

protected:
	UFUNCTION(BlueprintCallable)
	bool InitializeAbility(AActor* TargetActor);

protected:
	UPROPERTY(BlueprintReadOnly)
	TScriptInterface<ICYInteractable> Interactable;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> InteractableActor;
	
	UPROPERTY(BlueprintReadOnly)
	FCYInteractionInfo InteractionInfo;
};
