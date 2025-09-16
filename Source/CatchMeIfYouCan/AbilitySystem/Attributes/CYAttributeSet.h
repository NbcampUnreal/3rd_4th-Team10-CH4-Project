// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "CYAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

class UCYAbilitySystemComponent;

UCLASS()
class CATCHMEIFYOUCAN_API UCYAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCYAttributeSet();

	virtual UWorld* GetWorld() const override;

	UCYAbilitySystemComponent* GetCYAbilitySystemComponent() const;
};
