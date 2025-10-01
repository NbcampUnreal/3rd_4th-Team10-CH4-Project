// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedPlayerInput.h"
#include "CYEnhancedPlayerInput.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYEnhancedPlayerInput : public UEnhancedPlayerInput
{
	GENERATED_BODY()

public:
	UCYEnhancedPlayerInput();

public:
	void FlushPressedInput(UInputAction* InputAction);
	FKey GetKeyForAction(UInputAction* InputAction) const;
};
