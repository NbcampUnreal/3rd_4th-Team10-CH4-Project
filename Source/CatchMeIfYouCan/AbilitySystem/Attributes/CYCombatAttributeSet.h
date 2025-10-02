#pragma once

#include "CoreMinimal.h"
#include "CYAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CYCombatAttributeSet.generated.h"

class UCharacterMovementComponent;
class ACharacter;

UCLASS()
class CATCHMEIFYOUCAN_API UCYCombatAttributeSet : public UCYAttributeSet
{
	GENERATED_BODY()

public:
	UCYCombatAttributeSet();

	// 이동속도 속성
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UCYCombatAttributeSet, MoveSpeed)

	// 공격력 속성
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UCYCombatAttributeSet, AttackPower)

	// 네트워크 리플리케이션 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 속성 변경 전 검증
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	// 게임플레이 이펙트 실행 후 처리
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);
	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

private:
	void HandleMoveSpeedChange();
	// 이동 제한 적용
	void ApplyMovementRestrictions(ACharacter* Character, float Speed);
};