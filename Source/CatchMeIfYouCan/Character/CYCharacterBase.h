// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/CYAbilitySet.h"
#include "GameFramework/Character.h"
#include "CYCharacterBase.generated.h"

class UCYAbilitySet;
class UCYAbilitySystemComponent;

UCLASS(Abstract)
class CATCHMEIFYOUCAN_API ACYCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACYCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeAbilitySets();
	void RemoveAbilitySets();

protected:
	// 약참조로 ASC 관리(Player의 경우 PlayerState의 ASC를 사용, AI의 경우 Character의 ASC를 사용)
	TWeakObjectPtr<UCYAbilitySystemComponent> CYAbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CY|Abilities")
	TArray<TObjectPtr<UCYAbilitySet>> DefaultAbilitySets;

	UPROPERTY()
	TArray<FCYAbilitySet_GrantedHandles> GrantedAbilitySetHandles;
};
