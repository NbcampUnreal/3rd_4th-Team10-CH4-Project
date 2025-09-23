// Fill out your copyright notice in the Description page of Project Settings.


#include "CYUserWidget.h"

void UCYUserWidget::SetWidgetController(UCYWidgetController* InWidgetController)
{
	WidgetController = InWidgetController;
	OnWidgetControllerSet();
}
