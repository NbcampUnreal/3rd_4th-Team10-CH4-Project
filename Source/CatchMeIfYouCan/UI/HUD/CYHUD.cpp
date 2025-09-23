#include "CYHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/Widget/CYUserWidget.h"
#include "UI/WidgetController/CYOverlayWidgetController.h"

UCYOverlayWidgetController* ACYHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		// 위젯 컨트롤러가 없으면 지정된 클래스로 생성
		OverlayWidgetController = NewObject<UCYOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

void ACYHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass is not set in BP_HUD. Please fill it out."));
	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass is not set in BP_HUD. Please fill it out."));

	// Overlay 위젯 생성
	OverlayWidget = CreateWidget<UCYUserWidget>(GetWorld(), OverlayWidgetClass);

	// 위젯 컨트롤러 파라미터 구성
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	
	// 위젯 컨트롤러 가져오기(또는 생성)
	UCYOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	// 위젯에 위젯 컨트롤러 설정
	// 해당 시점에 OnWidgetControllerSet 함수가 호출됨
	// CYUserWidget을 상속받는 Overlay 블루프린트 위젯에서 OnWidgetControllerSet 이벤트 호출 가능
	// 자식이 없는 Overlay 위젯의 경우는 여기서 Attribute 변경에 대한 델리게이트를 바인딩하여 UI 업데이트 가능
	// 자식이 있는 Overlay 위젯 블루프린트 내 자식으로 가지고 있는 CYUserWidget타입의 위젯에 접근하여 WidgetController를 설정하고 여기서 또 OnWidgetControllerSet 함수를 호출
	// 해당 위젯에서 Attribute 변경에 대한 델리게이트를 바인딩하여 UI 업데이트 가능(Ex Overlay위젯의 자식으로 가지고 있는 HealthBar 위젯)
	OverlayWidget->SetWidgetController(WidgetController);
	
	// 위젯 컨트롤러가 초기값을 브로드캐스트하도록 호출
	WidgetController->BroadcastInitialValues();

	// 뷰포트에 위젯 추가
	OverlayWidget->AddToViewport();
}