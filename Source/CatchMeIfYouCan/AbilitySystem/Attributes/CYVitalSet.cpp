// Fill out your copyright notice in the Description page of Project Settings.


#include "CYVitalSet.h"

#include "Net/UnrealNetwork.h"

UCYVitalSet::UCYVitalSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
	
}

void UCYVitalSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UCYVitalSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UCYVitalSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCYVitalSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCYVitalSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UCYVitalSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCYVitalSet, Health, OldHealth);
}

void UCYVitalSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCYVitalSet, MaxHealth, OldMaxHealth);
}
