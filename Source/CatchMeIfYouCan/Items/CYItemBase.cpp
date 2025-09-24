#include "Items/CYItemBase.h"
#include "Character/CYPlayerCharacter.h"
#include "Player/CYPlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ACYItemBase::ACYItemBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    // Root Component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // Mesh Component
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
    ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    // Interaction Sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);

    // 기본값 설정
    bIsPickedUp = false;
    ItemCount = 1;
    MaxStackCount = 10;

	// 기본값: 모든 팀이 픽업 가능
	AllowedTeams.Add(ECYTeamRole::Cop);
	AllowedTeams.Add(ECYTeamRole::Robber);
}

void ACYItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ACYItemBase, bIsPickedUp);
    DOREPLIFETIME(ACYItemBase, ItemCount);
}

void ACYItemBase::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 충돌 이벤트 등록
    if (HasAuthority())
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYItemBase::OnSphereOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACYItemBase::OnSphereEndOverlap);
    }
}

void ACYItemBase::OnPickup(ACYPlayerCharacter* Character)
{
    if (!Character || bIsPickedUp || !HasAuthority()) return;

    bIsPickedUp = true;
    
    // 아이템을 숨기고 충돌 비활성화
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    
    UE_LOG(LogTemp, Warning, TEXT("Item picked up: %s"), *ItemName.ToString());
}

bool ACYItemBase::UseItem(ACYPlayerCharacter* Character)
{
    if (!Character) return false;
    
    // 기본 구현: 소모품 처리
    if (ItemType == EItemType::Consumable && ItemCount > 0)
    {
        ItemCount--;
        UE_LOG(LogTemp, Warning, TEXT("Used item: %s (Remaining: %d)"), 
               *ItemName.ToString(), ItemCount);
        return true;
    }
    
    return false;
}

bool ACYItemBase::CanStackWith(ACYItemBase* OtherItem) const
{
    if (!OtherItem) return false;
    
    // 같은 클래스이고, 스택 가능하고, 공간이 있어야 함
    return (GetClass() == OtherItem->GetClass()) && 
           (MaxStackCount > 1) && 
           (OtherItem->ItemCount < OtherItem->MaxStackCount);
}

void ACYItemBase::OnRep_ItemCount()
{
    // 아이템 수량 변경 시 UI 업데이트 등
    UE_LOG(LogTemp, Log, TEXT("Item count updated: %s x%d"), *ItemName.ToString(), ItemCount);
}

void ACYItemBase::OnRep_IsPickedUp()
{
	if (bIsPickedUp)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
	else
	{
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
	}
}

void ACYItemBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                  bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsPickedUp) return;

    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(OtherActor))
    {
        // UI 힌트 표시
        UE_LOG(LogTemp, Log, TEXT("Player near item: %s"), *ItemName.ToString());
    }
}

void ACYItemBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(OtherActor))
    {
        // UI 힌트 숨기기
        UE_LOG(LogTemp, Log, TEXT("Player left item area: %s"), *ItemName.ToString());
    }
}

bool ACYItemBase::CanBePickedUpBy(ACYPlayerCharacter* Character) const
{
	if (!Character) return false;
    
	// AllowedTeams가 비어있으면 모두 픽업 가능
	if (AllowedTeams.Num() == 0) return true;
    
	// PlayerState에서 팀 확인
	ACYPlayerState* PS = Character->GetPlayerState<ACYPlayerState>();
	if (!PS) return false;
    
	ECYTeamRole CharacterTeam = PS->GetTeamRole();
    
	// AllowedTeams에 캐릭터 팀이 포함되어 있는지 확인
	bool bCanPickup = AllowedTeams.Contains(CharacterTeam);
    
	if (!bCanPickup)
	{
		UE_LOG(LogTemp, Log, TEXT("Character team %s cannot pickup item %s (Allowed teams: %d)"), 
			CharacterTeam == ECYTeamRole::Cop ? TEXT("Cop") : TEXT("Robber"),
			*ItemName.ToString(),
			AllowedTeams.Num());
	}
    
	return bCanPickup;
}