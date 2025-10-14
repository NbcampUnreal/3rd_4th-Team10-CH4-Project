// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYWidgetController.h"
#include "Interaction/CYInteractionInfo.h"
#include "CYInteractionWidgetController.generated.h"

// 상호작용 메시지 타입
UENUM(BlueprintType)
enum class ECYInteractionMessageType : uint8
{
	Notice,      // 상호작용 가능 알림 (R키 프롬프트)
	Progress     // 홀딩 진행 상태
};

/**
 * UI 업데이트를 위한 상호작용 메시지 구조체
 */
USTRUCT(BlueprintType)
struct FCYInteractionMessage
{
	GENERATED_BODY()

	/** 메시지 타입 */
	UPROPERTY(BlueprintReadWrite, Category="Interaction")
	ECYInteractionMessageType MessageType = ECYInteractionMessageType::Notice;

	/** 상호작용 요청자 */
	UPROPERTY(BlueprintReadWrite, Category="Interaction")
	TObjectPtr<AActor> Instigator = nullptr;

	/** UI 새로고침 여부 */
	UPROPERTY(BlueprintReadWrite, Category="Interaction")
	bool bShouldRefresh = false;

	/** 활성 상태 전환 여부 */
	UPROPERTY(BlueprintReadWrite, Category="Interaction")
	bool bSwitchActive = false;

	/** 상호작용 정보 */
	UPROPERTY(BlueprintReadWrite, Category="Interaction")
	FCYInteractionInfo InteractionInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionMessageReceived, FCYInteractionMessage, Message);

/**
 * UI에서 상호작용 위젯을 제어하기 위한 컨트롤러 클래스
 */
UCLASS(BlueprintType, Blueprintable)
class CATCHMEIFYOUCAN_API UCYInteractionWidgetController : public UCYWidgetController
{
	GENERATED_BODY()

public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// 상호작용 메시지 브로드캐스트
	UFUNCTION(BlueprintCallable, Category="CY|Interaction")
	void BroadcastInteractionMessage(FCYInteractionMessage Message);

	// 상호작용 숨기기
	UFUNCTION(BlueprintCallable, Category="CY|Interaction")
	void HideInteraction();

	UPROPERTY(BlueprintAssignable, Category="CY|Interaction")
	FOnInteractionMessageReceived OnInteractionMessageReceived;
};
