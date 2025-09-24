#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CYWidgetController.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/**
 * 위젯 컨트롤러에 필요한 정보를 담을 구조체
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()
	
	FWidgetControllerParams() { }
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, AGameStateBase* GS)
		: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS), GameState(GS) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AGameStateBase> GameState = nullptr;
};

/**
 * 모든 위젯 컨트롤러의 부모 클래스
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYWidgetController : public UObject
{
	GENERATED_BODY()

public:
	// 위젯 컨트롤러 설정 함수
	UFUNCTION(BlueprintCallable, Category = "CY|Widget")
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

	//  위젯이 처음 생성될 때 초기값을 UI에 전달하기 위한 함수 (자식에서 구현)
	UFUNCTION(BlueprintCallable, Category="CY|Widget")
	virtual void BroadcastInitialValues() {}

	// Attribute 변경에 대한 콜백 함수들을 바인딩하는 함수 (자식에서 구현)
	UFUNCTION(BlueprintCallable, Category="CY|Widget")
	virtual void BindCallbacksToDependencies() {}

protected:
	
	UPROPERTY(BlueprintReadOnly, Category="CY|WidgetParams")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category="CY|WidgetParams")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category="CY|WidgetParams")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="CY|WidgetParams")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category="CY|WidgetParams")
	TObjectPtr<AGameStateBase> GameState;
};
