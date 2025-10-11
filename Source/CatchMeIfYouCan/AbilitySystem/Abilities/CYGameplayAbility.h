// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "UI/HUD/CYHUD.h"
#include "UI/WidgetController/CYWidgetController.h"
#include "CYGameplayAbility.generated.h"

class UInputAction;
class ACYCharacterBase;
class UCYWidgetController;

UENUM(BlueprintType)
enum class ECYAbilityActivationPolicy : uint8
{
	Manual,
	OnInputTriggered, //Input이 Trigger 되었을 경우 (Pressed/Released) 
	WhileInputActive, // Input이 Held되어 있을 경우 
	OnSpawn, // avatar가 생성되었을 경우 바로 할당(패시브 스킬 등) 
};

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UCYGameplayAbility();

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	ACYCharacterBase* GetCYCharacterFromActorInfo() const;
	ECYAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	UFUNCTION(BlueprintCallable, Category = "CY|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable)
	void FlushPressedInput(UInputAction* InputAction);

	/**
	 * HUD에서 특정 타입의 WidgetController를 가져오는 Template 함수
	 * @tparam T UCYWidgetController를 상속받는 타입
	 * @return 해당 타입의 WidgetController, 없으면 nullptr
	 */
	template<typename T>
	requires std::derived_from<T, UCYWidgetController>
	T* GetWidgetController() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CY|AbilityActivation")
	ECYAbilityActivationPolicy ActivationPolicy;
};

template <typename T>
requires std::derived_from<T, UCYWidgetController>
T* UCYGameplayAbility::GetWidgetController() const
{
	APlayerController* PC = Cast<APlayerController>(GetControllerFromActorInfo());
	if (!PC)
	{
		return nullptr;
	}

	ACYHUD* HUD = Cast<ACYHUD>(PC->GetHUD());
	if (!HUD)
	{
		return nullptr;
	}

	return HUD->GetWidgetController<T>(FWidgetControllerParams());
}
