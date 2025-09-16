// Fill out your copyright notice in the Description page of Project Settings.

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

