// Fill out your copyright notice in the Description page of Project Settings.


#include "CYAttributeSet.h"

#include "AbilitySystem/CYAbilitySystemComponent.h"

UCYAttributeSet::UCYAttributeSet()
{
}

UWorld* UCYAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UCYAbilitySystemComponent* UCYAttributeSet::GetCYAbilitySystemComponent() const
{
	return Cast<UCYAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
