// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CYUserWidget.generated.h"

class UCYWidgetController;


/**
 * 모든 UI 위젯의 부모 클래스로써 위젯 컨트롤러를 설정하여 MVC 패턴을 구현
 */
UCLASS(Abstract, BlueprintType)
class CATCHMEIFYOUCAN_API UCYUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "CY|Widget")
	void SetWidgetController(UCYWidgetController* InWidgetController);

	// WidgetController가 설정되었을 때 호출되는 함수로써 블루프린트에서 추가 로직 구현 가능
	UFUNCTION(BlueprintImplementable, Category = "CY|Widget")
	void OnWidgetControllerSet();

	UFUNCTION(BlueprintPure, Category="CY|Widget")
	UCYWidgetController* GetWidgetController() const { return WidgetController; }

	template<typename T>
	T* GetWidgetControllerTyped() const { return Cast<T>(WidgetController); }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CY|Widget")
	TObjectPtr<UCYWidgetController> WidgetController;
};
