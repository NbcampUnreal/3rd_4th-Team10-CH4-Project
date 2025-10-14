// Fill out your copyright notice in the Description page of Project Settings.


#include "CYInteractionWidgetController.h"

#include "Interaction/CYInteractionInfo.h"

void UCYInteractionWidgetController::BroadcastInteractionMessage(FCYInteractionMessage Message)
{
	OnInteractionMessageReceived.Broadcast(Message);
}

void UCYInteractionWidgetController::HideInteraction()
{
	FCYInteractionMessage EmptyMessage;
	OnInteractionMessageReceived.Broadcast(EmptyMessage);
}

void UCYInteractionWidgetController::BroadcastInitialValues()
{
	// 초기에는 상호작용 UI 숨김
	HideInteraction();
}

void UCYInteractionWidgetController::BindCallbacksToDependencies()
{
	// 필요 시 ASC 이벤트 바인딩
}