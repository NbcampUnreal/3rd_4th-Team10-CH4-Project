// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CYAbilitySet.generated.h"

class UGameplayEffect;
class UCYGameplayAbility;
class UAttributeSet;
struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;
class UCYAbilitySystemComponent;

/**
 * 어빌리티 set에서 부여할 어빌리티에 대한 데이터를 저장하는 구조체
 */
USTRUCT(BlueprintType)
struct FCYAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:

	// 부여할 어빌리티
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCYGameplayAbility> Ability = nullptr;

	// 어빌리티 레벨
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// 어빌리티에 대한 입력 태그
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 *	어빌리티 set에서 부여할 게임플레이 이펙트에 대한 데이터를 저장하는 구조체
 */
USTRUCT(BlueprintType)
struct FCYAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:

	// 부여할 게임플레이 이펙트
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// 이펙트 레벨
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/**
 *	어빌리티 set에서 부여할 어트리뷰트 세트에 대한 데이터를 저장하는 구조체
 */
USTRUCT(BlueprintType)
struct FCYAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// 부여할 어트리뷰트 세트
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;

};

/**
 * 어빌리티 셋에 의해 부여된 어빌리티, 게임플레이 이펙트, 어트리뷰트 세트에 대한 핸들을 저장하는 구조체
 */
USTRUCT(BlueprintType)
struct FCYAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	void TakeFromAbilitySystem(UCYAbilitySystemComponent* CYAbilitySystemComponent);

	
protected:

	// 부여된 Ability Spec Handles
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// 부여된 Gameplay Effect Handles
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// 부여된 AttributeSets 포인터
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 넘겨준 ASC에 어빌리티 셋을 부여하는 로직
	// 반환된 OutGrantedHandles를 사용하여 어빌리티 셋을 해제할 수 있다
	void GiveToAbilitySystem(UCYAbilitySystemComponent* CYAbilitySystemComponent, FCYAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;
	
protected:

	// 어빌리티 셋이 부여될 떄  부여할 어빌리티
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FCYAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// 어빌리티 셋이 부여될 떄 부여할 게임플레이 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FCYAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// 어빌리티 셋이 부여될 떄 부여할 어트리뷰트 세트
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FCYAbilitySet_AttributeSet> GrantedAttributeSets;
};
