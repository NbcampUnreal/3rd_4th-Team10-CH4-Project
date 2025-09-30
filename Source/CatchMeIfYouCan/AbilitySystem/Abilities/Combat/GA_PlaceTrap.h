#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GA_PlaceTrap.generated.h"

class ACYTrapBase;
class ACYItemBase;
class UAnimMontage;

UCLASS()
class CATCHMEIFYOUCAN_API UGA_PlaceTrap : public UCYGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlaceTrap();

	// 트랩 설치 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* PlaceTrapMontage;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 애니메이션 완료 콜백
	UFUNCTION()
	void OnPlaceTrapMontageCompleted();
	UFUNCTION()
	void OnPlaceTrapMontageCancelled();

private:
	// 실제 트랩 설치 로직
	void PerformTrapPlacement();
	// 어빌리티 스펙의 SourceObject에서 트랩 아이템 가져오기
	ACYTrapBase* GetTrapItemFromSource() const;
	// 인벤토리에서 트랩 아이템 찾기 (폴백용)
	ACYItemBase* FindTrapItemInInventory();
	// 아이템 정보로 실제 트랩 액터 생성하기
	ACYTrapBase* CreateTrapFromItem(ACYItemBase* TrapItem, const FVector& Location);
	// 트랩을 설치할 위치 계산 (라인 트레이스 사용)
	FVector CalculateSpawnLocation();
	// 사용한 트랩 아이템을 인벤토리에서 소모하기
	void ConsumeItemFromInventory(ACYItemBase* Item);

	// 쿨타운 상태 체크
	bool IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const;
	// 쿨타운 적용
	void ApplyTrapCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);

private:
	// 어빌리티 정보 캐시
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo;
	FGameplayAbilityActivationInfo CachedActivationInfo;
	
	// 트랩 설치 정보 캐시
	UPROPERTY()
	ACYTrapBase* CachedTrapItem;
	FVector CachedSpawnLocation;

	// AbilityTask 추가
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;
};