// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYPlayerCharacter.h"
#include "CYRobberCharacter.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYRobberCharacter : public ACYPlayerCharacter
{
	GENERATED_BODY()

public:

	ACYRobberCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Begin ICYInteractable Interface
	virtual FCYInteractionInfo GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;
	virtual bool CanInteraction(const FCYInteractionQuery& InteractionQuery) const override;
	// End ICYInteractable Interface

	bool CanArrested(const FCYInteractionQuery& InteractionQuery) const;

protected:

	virtual void BeginPlay() override;

protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Info")
	FCYInteractionInfo ArrestedInteractionInfo;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Stencil")
	int32 CustomDepthStencilValue = 255;
};
