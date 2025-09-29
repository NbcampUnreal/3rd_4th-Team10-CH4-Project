#include "CYAnimNotify_WeaponAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "GameFramework/Character.h"

UCYAnimNotify_WeaponAttack::UCYAnimNotify_WeaponAttack()
{
}

void UCYAnimNotify_WeaponAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// ASC 찾기
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!ASC)
	{
		return;
	}

	// GameplayEvent 발생
	FGameplayEventData EventData;
	EventData.Instigator = OwnerActor;
	EventData.Target = OwnerActor;

	ASC->HandleGameplayEvent(CYGameplayTags::Event_Combat_WeaponAttack, &EventData);

	UE_LOG(LogTemp, Warning, TEXT("Weapon attack notify triggered!"));
}