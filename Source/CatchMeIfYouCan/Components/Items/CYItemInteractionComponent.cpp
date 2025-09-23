#include "Components/Items/CYItemInteractionComponent.h"
#include "Items/CYItemBase.h"
#include "Items/Traps/CYTrapBase.h"
#include "Components/Items/CYInventoryComponent.h"
#include "Character/CYPlayerCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCYItemInteractionComponent::UCYItemInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYItemInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 서버에서만 타이머 시작
    if (GetOwner()->HasAuthority())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ItemCheckTimer,
            this,
            &UCYItemInteractionComponent::CheckForNearbyItems,
            CheckInterval,
            true // 반복
        );
    }
}

void UCYItemInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCYItemInteractionComponent, NearbyItem);
}

void UCYItemInteractionComponent::InteractWithNearbyItem()
{
    if (!NearbyItem) 
    {
        UE_LOG(LogTemp, Warning, TEXT("No nearby item to interact with"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Interacting with: %s"), *NearbyItem->GetName());
    ServerPickupItem(NearbyItem);
}

void UCYItemInteractionComponent::ServerPickupItem_Implementation(ACYItemBase* Item)
{
    if (!Item || !GetOwner()->HasAuthority() || Item->bIsPickedUp) return;
    
    UCYInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UCYInventoryComponent>();
    if (!InventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("No InventoryComponent found"));
        return;
    }
    
    // 인벤토리에 추가
    bool bAddedToInventory = InventoryComp->AddItem(Item);
    if (bAddedToInventory)
    {
        // 아이템 픽업 처리
        Item->OnPickup(Cast<ACYPlayerCharacter>(GetOwner()));
        UE_LOG(LogTemp, Warning, TEXT("Item picked up: %s"), *Item->ItemName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to add item to inventory"));
    }
}

void UCYItemInteractionComponent::CheckForNearbyItems()
{
    if (!GetOwner()) return;
    
    FVector PlayerLocation = GetOwner()->GetActorLocation();
    ACYItemBase* ClosestItem = nullptr;
    float ClosestDistance = FLT_MAX;
    
    // 모든 아이템 찾기
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACYItemBase::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        ACYItemBase* Item = Cast<ACYItemBase>(Actor);
        if (!Item || Item->bIsPickedUp) continue;
        
        // 트랩의 경우 맵에 배치된 것만 픽업 가능
        if (ACYTrapBase* Trap = Cast<ACYTrapBase>(Item))
        {
            if (Trap->TrapState != ETrapState::MapPlaced) continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Item->GetActorLocation());
        if (Distance < InteractionRange && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestItem = Item;
        }
    }
    
    // 변경사항이 있을 때만 업데이트
    if (NearbyItem != ClosestItem)
    {
        NearbyItem = ClosestItem;
        OnRep_NearbyItem();
    }
}

void UCYItemInteractionComponent::OnRep_NearbyItem()
{
	// 이전 아이템 하이라이트 해제
	if (PreviousNearbyItem && PreviousNearbyItem->ItemMesh)
	{
		// 기존 Material이 Dynamic Material인지 확인 후 파라미터 설정
		if (UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(PreviousNearbyItem->ItemMesh->GetMaterial(0)))
		{
			DynMat->SetVectorParameterValue(TEXT("HighlightColor"), FLinearColor::Black);
		}
	}
    
	// 새 아이템 하이라이트
	if (NearbyItem && NearbyItem->ItemMesh)
	{
		UMaterialInterface* OriginalMaterial = NearbyItem->ItemMesh->GetMaterial(0);
		if (OriginalMaterial)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(OriginalMaterial, this);
			NearbyItem->ItemMesh->SetMaterial(0, DynMat);
			DynMat->SetVectorParameterValue(TEXT("HighlightColor"), FLinearColor::White);
		}
	}
    
	PreviousNearbyItem = NearbyItem;
}