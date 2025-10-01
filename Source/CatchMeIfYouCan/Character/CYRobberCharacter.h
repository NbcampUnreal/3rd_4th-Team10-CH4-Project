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

	// TODO : 추후 경찰이나 도둑, 다른 캐릭터 타입에 대해 해당 함수 필요할 경우 CharacterBase로 옮기거나 다른 최적의 위치나 방법 고민
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
