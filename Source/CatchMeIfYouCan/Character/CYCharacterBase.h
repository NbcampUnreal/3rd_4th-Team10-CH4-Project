// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/CYAbilitySet.h"
#include "GameFramework/Character.h"
#include "Interaction/CYInteractable.h"
#include "CYCharacterBase.generated.h"

class UCYAbilitySet;
class UCYAbilitySystemComponent;
// Item 컴포넌트 추가
class UCYInventoryComponent;
class UCYItemInteractionComponent;
class UCYWeaponComponent;
class UCYCombatAttributeSet;

class UMotionWarpingComponent;

UCLASS(Abstract)
class CATCHMEIFYOUCAN_API ACYCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICYInteractable
{
	GENERATED_BODY()

public:
	ACYCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// Item 컴포넌트 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CY|Components")
	UCYInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CY|Components")
	UCYItemInteractionComponent* ItemInteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CY|Components")
	UCYWeaponComponent* WeaponComponent;
	
	// Item 입력 함수들 추가
	UFUNCTION(BlueprintCallable, Category = "CY|Input")
	void InteractPressed();

	UFUNCTION(BlueprintCallable, Category = "CY|Input")
	void AttackPressed();

	UFUNCTION(BlueprintCallable, Category = "CY|Input")
	void UseInventorySlot(int32 SlotIndex);
	
	void TryInitializeAbilitySetsWithPawnData();

	UFUNCTION(BlueprintCallable, Category = "CY|AbilitySystem")
	void AddGameplayTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, Category = "CY|AbilitySystem")
	void RemoveGameplayTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, Category = "CY|AbilitySystem")
	bool HasGameplayTag(const FGameplayTag& Tag) const;

	UFUNCTION(BlueprintCallable, Category = "CY|Equipment")
	USkeletalMeshComponent* GetHelmetMesh() const { return HelmetMesh; }

	UFUNCTION(BlueprintCallable, Category = "CY|Equipment")
	USkeletalMeshComponent* GetEyewearMesh() const { return EyewearMesh; }

	UFUNCTION(BlueprintCallable, Category = "CY|Equipment")
	USkeletalMeshComponent* GetChestMesh() const { return ChestMesh; }

	UFUNCTION(BlueprintCallable, Category = "CY|Equipment")
	USkeletalMeshComponent* GetLegsMesh() const { return LegsMesh; }

	UFUNCTION(BlueprintCallable, Category = "CY|Equipment")
	USkeletalMeshComponent* GetFootwearMesh() const { return FootwearMesh; }

	UFUNCTION(BlueprintCallable, Category = "CY|Animation")
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool IsPawnDataReady() const;
	
	void InitializeAbilitySets();
	void RemoveAbilitySets();

	void SyncInteractCapsuleSizeToRootCapsule() const;

protected:
	UPROPERTY(VisibleAnywhere, Category="CY|Interaction")
	UCapsuleComponent* InteractCapsule;

	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float InteractCapsuleRadiusOffset = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category="CY|Interaction")
	float InteractCapsuleHalfHeightOffset = 0.f; 
	
	// 약참조로 ASC 관리(Player의 경우 PlayerState의 ASC를 사용, AI의 경우 Character의 ASC를 사용)
	TWeakObjectPtr<UCYAbilitySystemComponent> CYAbilitySystemComponent;
	
	UPROPERTY()
	TArray<FCYAbilitySet_GrantedHandles> GrantedAbilitySetHandles;

	// 초기화 상태 추적
	bool bAbilitySetsInitialized = false;

private:
	// 헬멧
	UPROPERTY(VisibleAnywhere, Category = "CY|Equipment")
	USkeletalMeshComponent* HelmetMesh;

	// 안경/고글/눈
	UPROPERTY(VisibleAnywhere, Category = "CY|Equipment")
	USkeletalMeshComponent* EyewearMesh; 

	// 상체
	UPROPERTY(VisibleAnywhere, Category = "CY|Equipment")
	USkeletalMeshComponent* ChestMesh;    

	// 하체
	UPROPERTY(VisibleAnywhere, Category = "CY|Equipment")
	USkeletalMeshComponent* LegsMesh;    

	// 신발
	UPROPERTY(VisibleAnywhere, Category = "CY|Equipment")
	USkeletalMeshComponent* FootwearMesh;

	// Motion Warping 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "CY|Animation")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
};
