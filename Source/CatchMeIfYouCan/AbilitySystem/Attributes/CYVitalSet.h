// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "CYVitalSet.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class CATCHMEIFYOUCAN_API UCYVitalSet : public UCYAttributeSet
{
	GENERATED_BODY()

public:
	UCYVitalSet();
	
	ATTRIBUTE_ACCESSORS(UCYVitalSet, Health);
	ATTRIBUTE_ACCESSORS(UCYVitalSet, MaxHealth);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "CY|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "CY|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;
};
