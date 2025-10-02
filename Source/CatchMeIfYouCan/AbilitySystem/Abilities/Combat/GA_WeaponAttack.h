#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GA_WeaponAttack.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;

UCLASS()
class CATCHMEIFYOUCAN_API UGA_WeaponAttack : public UCYGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponAttack();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

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

private:
	// 실제 공격 로직 수행
	void PerformAttack();
	// 맞은 대상 처리
	void ProcessHitTarget(const FHitResult& HitResult);
	// 데미지 적용
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult);
	// 쿨다운 상태 체크
	bool IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const;
	// 쿨다운 적용
	void ApplyWeaponCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);
	// 근접 공격 디버그 시각화
	void DrawMeleeAttackDebug(const FVector& Start, const FVector& End, float Radius, bool bHit);

	// AbilityTask 콜백들
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnAttackEventReceived(FGameplayEventData Payload);

private:
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* AttackEventTask;
	
	// // 어빌리티 정보 저장 (몽타주 완료 후 사용)
	// FGameplayAbilitySpecHandle CachedHandle;
	// const FGameplayAbilityActorInfo* CachedActorInfo;
	// FGameplayAbilityActivationInfo CachedActivationInfo;
	//
	// bool bAttackExecuted = false;

	
};