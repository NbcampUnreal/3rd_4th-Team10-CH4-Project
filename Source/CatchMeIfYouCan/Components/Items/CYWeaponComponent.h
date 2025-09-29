#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CYWeaponComponent.generated.h"

class ACYWeaponBase;
class UCYAbilitySystemComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChanged, ACYWeaponBase*, OldWeapon, ACYWeaponBase*, NewWeapon);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHMEIFYOUCAN_API UCYWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYWeaponComponent();

	// 현재 장착된 무기
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Weapon")
	ACYWeaponBase* CurrentWeapon;

	// 무기 장착 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponSocketName = TEXT("RightHand");

	// 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeaponChanged OnWeaponChanged;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeapon(ACYWeaponBase* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool UnequipWeapon();

	// 애니메이션 블루프린트 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> UnarmedAnimBP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
	TMap<TSubclassOf<ACYWeaponBase>, TSubclassOf<UAnimInstance>> WeaponAnimBPMap;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UCYAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	USkeletalMeshComponent* GetOwnerMesh() const;
	void AttachWeaponToOwner(ACYWeaponBase* Weapon);

	// 애니메이션 블루프린트 변경
	void UpdateAnimationBlueprint();
};