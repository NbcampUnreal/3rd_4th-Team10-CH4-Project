#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GA_Stunned.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API UGA_Stunned : public UCYGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Stunned();

	// 쓰러지는 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* FallingMontage;

	// 쓰러져있는 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* LyingMontage;

	// Stunned 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stunned")
	float StunnedDuration = 5.0f;

	// 회복할 HP 양
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stunned")
	float RecoveryHealth = 1.0f;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	FTimerHandle RecoveryTimerHandle;
	
	UFUNCTION()
	void RecoverFromStun();

	// 몽타주 콜백
	UFUNCTION()
	void OnFallingMontageCompleted();

	UFUNCTION()
	void OnFallingMontageCancelled();
	
	// 캐시된 정보
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo;
	FGameplayAbilityActivationInfo CachedActivationInfo;

	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* FallingMontageTask;

	UPROPERTY()
	UAnimInstance* CachedAnimInstance;
};