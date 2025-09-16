// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "CYPlayerState.generated.h"


class UCYVitalSet;
class UCYAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACYPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "CY|PlayerState")
	UCYAbilitySystemComponent* GetCYAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual void PostInitializeComponents() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "CY|PlayerState")
	TObjectPtr<UCYAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UCYVitalSet> VitalSet;
};
