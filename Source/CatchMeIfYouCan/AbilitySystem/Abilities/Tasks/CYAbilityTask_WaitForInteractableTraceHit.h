// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Interaction/CYInteractionQuery.h"
#include "CYAbilityTask_WaitForInteractableTraceHit.generated.h"

struct FCYInteractionInfo;
class ICYInteractable;

/**
 * 상호작용 가능한 객체들의 변화를 알리는 델리게이트
 * @param InteractableInfos 현재 감지된 상호작용 정보 배열
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, const TArray<FCYInteractionInfo>&, InteractableInfos);

/**
 * 플레이어의 시선 방향으로 주기적으로 레이캐스트를 수행하여
 * 상호작용 가능한 객체들을 감지하고 추적하는 어빌리티 태스크
 * 주요 기능:
 * - 카메라 시점 기반 정밀한 상호작용 대상 감지
 * - 플레이어 위치 중심의 구체 범위 내 제한
 * - 상호작용 가능 객체의 하이라이트 효과 관리
 * - 상호작용 정보 변화 감지 및 알림
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYAbilityTask_WaitForInteractableTraceHit : public UAbilityTask
{
	GENERATED_BODY()

public:
	UCYAbilityTask_WaitForInteractableTraceHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UCYAbilityTask_WaitForInteractableTraceHit* WaitForInteractableTraceHit(UGameplayAbility* OwningAbility, FCYInteractionQuery InteractionQuery, ECollisionChannel TraceChannel, FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionTraceRange = 100.f, float InteractionTraceRate = 0.1f, bool bShowDebug = false);

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	void PerformTrace();

	/**
	 * 플레이어 컨트롤러의 카메라 시점을 기반으로 레이캐스트 종료점을 계산
	 * 카메라 방향과 플레이어 위치 기준 구체 범위를 고려한 정밀한 타겟팅
	 * @param InSourceActor 소스 액터 (플레이어)
	 * @param Params 콜리전 쿼리 파라미터
	 * @param TraceStart 레이캐스트 시작점 (플레이어 위치)
	 * @param MaxRange 최대 상호작용 거리
	 * @param OutTraceEnd [출력] 계산된 레이캐스트 종료점
	 * @param bIgnorePitch 피치 무시 여부 (기본값: false)
	 */
	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	/**
	 * 카메라 레이를 플레이어 중심의 구체 범위 내로 제한하는 함수
	 * 카메라가 플레이어에서 멀리 떨어져 있어도 상호작용 범위를 벗어나지 않도록 보장
	 * @param CameraLocation 카메라 위치
	 * @param CameraDirection 카메라 방향
	 * @param AbilityCenter 어빌리티 중심점 (플레이어 위치)
	 * @param AbilityRange 어빌리티 범위 (구체 반지름)
	 * @param OutClippedPosition [출력] 클리핑된 위치
	 * @return 클리핑이 성공했는지 여부
	 */
	bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& OutClippedPosition) const;

	
	void LineTrace(const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHitResult) const;

	/**
	 * 감지된 상호작용 가능한 객체들로부터 상호작용 정보를 수집하고 업데이트
	 * 이전 정보와 비교하여 변화가 있을 때만 델리게이트를 호출
	 * @param InteractQuery 상호작용 쿼리
	 * @param Interactables 감지된 상호작용 가능한 객체들
	 */
	void UpdateInteractionInfos(const FCYInteractionQuery& InteractQuery, const TArray<TScriptInterface<ICYInteractable>>& Interactables);

	/**
	 * 상호작용 가능한 객체들의 하이라이트 효과를 관리
	 * CustomDepth 렌더링을 사용하여 아웃라인 효과 구현
	 * @param InteractionInfos 하이라이트할 상호작용 정보들
	 * @param bShouldHighlight 하이라이트 활성화 여부
	 */
	void HighlightInteractables(const TArray<FCYInteractionInfo>& InteractionInfos, bool bShouldHighlight);

public:
	UPROPERTY(BlueprintAssignable)
	FOnInteractableChanged InteractableChanged;

private:
	UPROPERTY()
	FCYInteractionQuery InteractionQuery;

	/** 레이캐스트 시작 위치 정보 */
	UPROPERTY()
	FGameplayAbilityTargetingLocationInfo StartLocation;

	ECollisionChannel TraceChannel = ECC_Visibility;
	float InteractionTraceRange = 100.f;
	float InteractionTraceRate = 0.1f;
	bool bShowDebug = false;
	
	FTimerHandle TraceTimerHandle;

	/** 현재 감지된 상호작용 정보들 (변화 감지용) */
	TArray<FCYInteractionInfo> CurrentInteractionInfos;
};
