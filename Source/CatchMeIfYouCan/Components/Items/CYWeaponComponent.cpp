#include "Components/Items/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UCYWeaponComponent::UCYWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCYWeaponComponent, CurrentWeapon);
}

bool UCYWeaponComponent::EquipWeapon(ACYWeaponBase* Weapon)
{
	if (!Weapon || !GetOwner()->HasAuthority()) return false;

	if (CurrentWeapon)
	{
		UnequipWeapon();
	}

	CurrentWeapon = Weapon;
	AttachWeaponToOwner(Weapon);
    
	Weapon->SetActorHiddenInGame(false);
    
	// 충돌 비활성화
	if (Weapon->ItemMesh)
	{
		Weapon->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (Weapon->InteractionSphere)
	{
		Weapon->InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UpdateAnimationBlueprint();
    
	// 네트워크 업데이트 강제
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
	
	OnWeaponChanged.Broadcast(nullptr, CurrentWeapon);
    
	UE_LOG(LogTemp, Warning, TEXT("Weapon equipped: %s"), *Weapon->ItemName.ToString());
	return true;
}

bool UCYWeaponComponent::UnequipWeapon()
{
	if (!CurrentWeapon || !GetOwner()->HasAuthority()) return false;

	ACYWeaponBase* OldWeapon = CurrentWeapon;
    
	// 무기를 숨기기 (장착 해제 시 인벤토리에 있으므로)
	OldWeapon->SetActorHiddenInGame(true);
    
	// 부착 해제
	OldWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CurrentWeapon = nullptr;

	// 애니메이션 블루프린트를 맨손 상태로 변경
	UpdateAnimationBlueprint();

	OnWeaponChanged.Broadcast(OldWeapon, nullptr);
    
	UE_LOG(LogTemp, Warning, TEXT("Weapon unequipped: %s"), *OldWeapon->ItemName.ToString());
	return true;
}

void UCYWeaponComponent::UpdateAnimationBlueprint()
{
	USkeletalMeshComponent* OwnerMesh = GetOwnerMesh();
	if (!OwnerMesh)
	{
		return;
	}

	TSubclassOf<UAnimInstance> TargetAnimBP = UnarmedAnimBP; // 기본값: 맨손

	if (CurrentWeapon)
	{
		// 무기별 애니메이션 블루프린트 찾기
		TSubclassOf<ACYWeaponBase> WeaponClass = CurrentWeapon->GetClass();
		if (TSubclassOf<UAnimInstance>* FoundAnimBP = WeaponAnimBPMap.Find(WeaponClass))
		{
			TargetAnimBP = *FoundAnimBP;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No animation blueprint found for weapon: %s"), 
				   *WeaponClass->GetName());
		}
	}

	if (TargetAnimBP)
	{
		// Link Anim Class Layers 사용
		OwnerMesh->LinkAnimClassLayers(TargetAnimBP);
        
		UE_LOG(LogTemp, Warning, TEXT("Animation blueprint changed to: %s"), 
			   *TargetAnimBP->GetName());
	}
}

UCYAbilitySystemComponent* UCYWeaponComponent::GetOwnerAbilitySystemComponent() const
{
    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
    {
        return Cast<UCYAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
    }
    return nullptr;
}

USkeletalMeshComponent* UCYWeaponComponent::GetOwnerMesh() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetMesh();
    }
    return nullptr;
}

void UCYWeaponComponent::AttachWeaponToOwner(ACYWeaponBase* Weapon)
{
    if (!Weapon) return;

    USkeletalMeshComponent* OwnerMesh = GetOwnerMesh();
    if (OwnerMesh)
    {
        Weapon->AttachToComponent(
            OwnerMesh,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            WeaponSocketName
        );
        
        UE_LOG(LogTemp, Log, TEXT("Weapon attached to socket: %s"), *WeaponSocketName.ToString());
    }
}

void UCYWeaponComponent::OnRep_CurrentWeapon()
{
	OnWeaponChanged.Broadcast(nullptr, CurrentWeapon);
    
	if (CurrentWeapon)
	{
		AttachWeaponToOwner(CurrentWeapon);
		CurrentWeapon->SetActorHiddenInGame(false);  // 클라이언트에서도 보이게
		UE_LOG(LogTemp, Log, TEXT("Client weapon replicated: %s"), *CurrentWeapon->ItemName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Client weapon unequipped"));
	}

	// 클라이언트에서도 애니메이션 블루프린트 변경
	UpdateAnimationBlueprint();
}