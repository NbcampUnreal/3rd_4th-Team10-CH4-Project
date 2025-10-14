// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYInteractable.h"
#include "GameFramework/Actor.h"
#include "CYWorldInteractable.generated.h"

/**
 * 월드에 배치되는 상호작용 가능한 객체들의 기본 클래스
 * AActor를 상속받아 월드에 배치 가능하며, ICYInteractable 인터페이스를 구현
 * 사용 예시: 문, 상자, 레버, 버튼, 픽업 아이템 등
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYWorldInteractable : public AActor, public ICYInteractable
{
	GENERATED_BODY()

public:
	ACYWorldInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool CanInteraction(const FCYInteractionQuery& InteractionQuery) const override;

	UFUNCTION()
	virtual void OnRep_WasConsumed();
	
public:
	/**
	 * 홀딩 상호작용이 시작될 때 호출되는 함수
	 * 플레이어가 상호작용 키를 누르고 홀딩을 시작할 때 실행
	 * @param Interactor 상호작용을 시작한 액터 (보통 플레이어)
	 */
	UFUNCTION(BlueprintCallable)
	virtual void OnInteractActiveStarted(AActor* Interactor);

	
	UFUNCTION(BlueprintImplementableEvent, DisplayName="OnInteractActiveStarted")
	void K2_OnInteractActiveStarted(AActor* Interactor);

	/**
	 * 홀딩 상호작용이 종료될 때 호출되는 함수
	 * 플레이어가 키를 떼거나, 홀딩이 완료되거나, 취소될 때 실행
	 * @param Interactor 상호작용을 종료한 액터 (보통 플레이어)
	 */
	UFUNCTION(BlueprintCallable)
	virtual void OnInteractActiveEnded(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="OnInteractActiveEnded")
	void K2_OnInteractActiveEnded(AActor* Interactor);

	/**
	 * 상호작용이 성공적으로 완료될 때 호출되는 함수
	 * 홀딩이 끝까지 완료되어 실제 상호작용 효과가 발생할 때 실행
	 * 일회성 객체의 경우 소모 처리 및 다른 상호작용자들의 상호작용 취소
	 */
	UFUNCTION(BlueprintCallable)
	virtual void OnInteractionSuccess(AActor* Interactor);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName="OnInteractionSuccess")
	void K2_OnInteractionSuccess(AActor* Interactor);
	
protected:
	/**
	 * 일회성 상호작용 여부
	 * true = 한 번 사용하면 더 이상 상호작용 불가능 (소모품 아이템 등)
	 * false = 반복 사용 가능 (문, 레버 등)
	 */
	UPROPERTY(EditDefaultsOnly)
	bool bShouldConsume = false;

	/**
	 * bShouldConsume이 true일 때 사용되며, 한 번 true가 되면 상호작용 불가능
	 */
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=OnRep_WasConsumed)
	bool bWasConsumed = false;

	/**
	 * 현재 이 객체와 상호작용 중인 액터들의 캐시
	 * 다중 플레이어가 동시에 상호작용할 때 관리용
	 */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> CachedInteractors;
};
