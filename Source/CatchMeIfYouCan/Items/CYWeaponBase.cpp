#include "Items/CYWeaponBase.h"
#include "Character/CYPlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

ACYWeaponBase::ACYWeaponBase()
{
	ItemName = FText::FromString("Base Weapon");
	ItemType = EItemType::Weapon;
	MaxStackCount = 1; // 무기는 스택 불가
    
	OwningCharacter = nullptr;
}

bool ACYWeaponBase::UseItem(ACYPlayerCharacter* Character)
{
	if (!Character) return false;
    
	// 무기 사용 = 장착
	Equip(Character);
	return true;
}

void ACYWeaponBase::Equip(ACYPlayerCharacter* Character)
{
	if (!Character || !HasAuthority()) return;

	OwningCharacter = Character;
    
	// 무기를 캐릭터에 부착
	if (Character->GetMesh())
	{
		AttachToComponent(
			Character->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			TEXT("hand_r")  // 오른손 소켓
		);
	}

	// 충돌 비활성화 (장착된 무기는 월드와 충돌하지 않음)
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
	// 보이게 하기
	SetActorHiddenInGame(false);

	UE_LOG(LogTemp, Warning, TEXT("Weapon equipped: %s"), *ItemName.ToString());
}

void ACYWeaponBase::Unequip()
{
	if (!OwningCharacter || !HasAuthority()) return;

	// 부착 해제
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    
	OwningCharacter = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("Weapon unequipped: %s"), *ItemName.ToString());
}