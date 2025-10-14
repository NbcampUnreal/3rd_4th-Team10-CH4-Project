// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CYHUD.generated.h"

class UCYWidgetController;
struct FWidgetControllerParams;
class UAbilitySystemComponent;
class UAttributeSet;
class UCYUserWidget;
class UCYOverlayWidgetController;
class UCYInteractionWidgetController;
/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYHUD : public AHUD
{
	GENERATED_BODY()

public:

	// Overlay 위젯 컨트롤러를 가져오는 함수 (없으면 생성)
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCYOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

	// Interaction 위젯 컨트롤러를 가져오는 함수 (없으면 생성)
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCYInteractionWidgetController* GetInteractionWidgetController(const FWidgetControllerParams& WCParams);

	// HUD 위젯 초기화
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, AGameStateBase* GS);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCYWidgetController* GetWidgetController(TSubclassOf<UCYWidgetController> ControllerClass, const FWidgetControllerParams& WCParams);

	template<typename T>
	requires std::derived_from<T, UCYWidgetController>
	T* GetWidgetController(const FWidgetControllerParams& WCParams)
	{
		return Cast<T>(GetWidgetController(T::StaticClass(), WCParams));
	}

private:
	// Overlay 위젯 컨트롤러 인스턴스
	UPROPERTY()
	TObjectPtr<UCYOverlayWidgetController> OverlayWidgetController;

	// Overlay 위젯 컨트롤러의 블루프린트 클래스
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCYOverlayWidgetController> OverlayWidgetControllerClass;

	// Overlay 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UCYUserWidget> OverlayWidget;

	// Overlay 위젯의 블루프린트 클래스
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCYUserWidget> OverlayWidgetClass;

	// Interaction 관련
	UPROPERTY()
	TObjectPtr<UCYInteractionWidgetController> InteractionWidgetController;
    
	UPROPERTY(EditAnywhere, Category="CY|Interaction")
	TSubclassOf<UCYInteractionWidgetController> InteractionWidgetControllerClass;
	
	UPROPERTY()
	TMap<TSubclassOf<UCYWidgetController>, UCYWidgetController*> WidgetControllers;
};
