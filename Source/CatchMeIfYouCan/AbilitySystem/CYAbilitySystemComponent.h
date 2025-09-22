#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CYAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHMEIFYOUCAN_API UCYAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UCYAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	void AbilityInputTagStarted(const FGameplayTag& InputTag);
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	// Item 파트
	UFUNCTION(BlueprintCallable, Category = "Abilities|Items")
	FGameplayAbilitySpecHandle GiveItemAbility(TSubclassOf<class UGameplayAbility> AbilityClass, int32 Level = 1);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Items")
	void RemoveItemAbility(FGameplayAbilitySpecHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Items")
	bool TryActivateAbilityByTag(FGameplayTag AbilityTag);

	// SourceObject와 함께 어빌리티 활성화 (트랩 시스템용)
	bool TryActivateAbilityByTagWithSource(FGameplayTag AbilityTag, UObject* SourceObject);

protected:
	virtual void AbilitySpecInputStarted(FGameplayAbilitySpec& Spec);
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

protected:
	TArray<FGameplayAbilitySpecHandle> InputStartedSpecHandles;

	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	
};

