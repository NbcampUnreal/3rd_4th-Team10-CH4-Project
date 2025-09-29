// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYPlayerCharacter.h"
#include "CYCopCharacter.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYCopCharacter : public ACYPlayerCharacter
{
	GENERATED_BODY()

public:

	ACYCopCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Begin ICYInteractable Interface
	virtual FCYInteractionInfo GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;
	virtual bool CanInteraction(const FCYInteractionQuery& InteractionQuery) const override;
	// End ICYInteractable Interface

protected:
	
	virtual void BeginPlay() override;

};
