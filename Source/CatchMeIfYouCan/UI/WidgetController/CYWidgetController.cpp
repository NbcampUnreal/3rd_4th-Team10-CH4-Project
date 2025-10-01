// Fill out your copyright notice in the Description page of Project Settings.


#include "CYWidgetController.h"

void UCYWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
	GameState = WCParams.GameState;
}
