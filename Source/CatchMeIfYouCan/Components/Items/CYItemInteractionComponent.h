#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CYItemInteractionComponent.generated.h"

class UWidgetComponent;
class ACYItemBase;
class UCYInventoryComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHMEIFYOUCAN_API UCYItemInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYItemInteractionComponent();

	// 위젯 회전을 위한 Tick
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 상호작용 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 200.0f;

	// 현재 근처 아이템
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Interaction")
	ACYItemBase* NearbyItem;

	// 클라이언트 아이템 하이라이트
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	ACYItemBase* LocalNearbyItem;

	// E키로 호출되는 함수
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithNearbyItem();

	UFUNCTION(Server, Reliable, Category = "Interaction")
	void ServerPickupItem(ACYItemBase* Item);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI")
	TSubclassOf<UUserWidget> InteractionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI")
	FVector WidgetOffset = FVector(0.f, 0.f, 100.f); 

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 주기적으로 근처 아이템 검사
	UFUNCTION()
	void CheckForNearbyItems();

	// 로컬 전용 하이라이트 함수들
	void UpdateLocalHighlight();
	void ApplyHighlight(ACYItemBase* Item);
	void RemoveHighlight(ACYItemBase* Item);

private:
	FTimerHandle ItemCheckTimer;
	float CheckInterval = 0.1f; // 0.1초마다 체크

	// 하이라이트 정보 저장
	UPROPERTY()
	ACYItemBase* CurrentHighlightedItem = nullptr;

	UPROPERTY()
	UWidgetComponent* CurrentInteractionWidget = nullptr;

	void CreateInteractionWidget(ACYItemBase* Item);
	void RemoveInteractionWidget();
};