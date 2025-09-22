#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CYInventoryComponent.generated.h"

class ACYItemBase;
class ACYWeaponBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, int32, SlotIndex, ACYItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeldItemChanged, ACYItemBase*, OldItem, ACYItemBase*, NewItem);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHMEIFYOUCAN_API UCYInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCYInventoryComponent();

    // 슬롯 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 WeaponSlotCount = 3;  // 1~3번 키

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 ItemSlotCount = 6;    // 4~9번 키

    // 슬롯 배열 (네트워크 동기화)
    UPROPERTY(ReplicatedUsing = OnRep_WeaponSlots, BlueprintReadOnly, Category = "Inventory")
    TArray<ACYItemBase*> WeaponSlots;

    UPROPERTY(ReplicatedUsing = OnRep_ItemSlots, BlueprintReadOnly, Category = "Inventory")
    TArray<ACYItemBase*> ItemSlots;

    // 무기가 아닌 현재 들고 있는 아이템
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHeldItem, BlueprintReadOnly, Category = "Inventory")
    ACYItemBase* CurrentHeldItem;

    // 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHeldItemChanged OnHeldItemChanged;

	// 아이템을 인벤토리에 추가
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(ACYItemBase* Item);

	// 특정 슬롯의 아이템 가져오기
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    ACYItemBase* GetItem(int32 SlotIndex) const;

	// 특정 슬롯의 아이템을 손에 들기/무기 장착
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HoldItem(int32 SlotIndex);

	// 현재 들고 있는 아이템 사용하기
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseHeldItem();

	// 서버에서 아이템 들기 처리
    UFUNCTION(Server, Reliable, Category = "Inventory")
    void ServerHoldItem(int32 SlotIndex);

	// 서버에서 들고 있는 아이템 사용 처리
    UFUNCTION(Server, Reliable, Category = "Inventory")
    void ServerUseHeldItem();

	// 화면에 인벤토리 상태 표시 (디버그용)
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowInventoryDebug();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 네트워크 동기화
    UFUNCTION()
    void OnRep_WeaponSlots();

    UFUNCTION()
    void OnRep_ItemSlots();

    UFUNCTION()
    void OnRep_CurrentHeldItem();

	// 무기를 무기 슬롯에 추가
    bool AddWeapon(ACYItemBase* Weapon);
	// 아이템을 스택킹과 함께 아이템 슬롯에 추가
    bool AddItemWithStacking(ACYItemBase* Item);

	// 빈 무기 슬롯 찾기
    int32 FindEmptyWeaponSlot() const;
	// 빈 아이템 슬롯 찾기
    int32 FindEmptyItemSlot() const;
	// 스택 가능한 아이템 슬롯 찾기
    int32 FindStackableItemSlot(ACYItemBase* Item) const;
	// 기존 아이템과 스택 시도
    bool TryStackWithExistingItem(ACYItemBase* Item);

	// 아이템을 소켓에 부착
    void AttachItemToHand(ACYItemBase* Item);
	// 아이템을 소켓에서 해제
    void DetachItemFromHand(ACYItemBase* Item);

    // 슬롯 인덱스 변환
    bool IsWeaponSlot(int32 SlotIndex) const { return SlotIndex >= 1 && SlotIndex <= 3; }
    bool IsItemSlot(int32 SlotIndex) const { return SlotIndex >= 4 && SlotIndex <= 9; }
    int32 WeaponSlotToIndex(int32 SlotIndex) const { return SlotIndex - 1; }
    int32 ItemSlotToIndex(int32 SlotIndex) const { return SlotIndex - 4; }

private:
    // 중복 실행 방지
    UPROPERTY(Replicated)
    bool bIsProcessingUse = false;
};