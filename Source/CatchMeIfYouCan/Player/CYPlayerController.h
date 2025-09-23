// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CYPlayerController.generated.h"

class ACYPlayerState;
class UCYAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	ACYPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "CY|PlayerController")
	ACYPlayerState* GetCYPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "CY|PlayerController")
	UCYAbilitySystemComponent* GetCYAbilitySystemComponent() const;

protected:

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
};