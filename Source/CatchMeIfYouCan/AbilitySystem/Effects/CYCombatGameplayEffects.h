#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CYCombatGameplayEffects.generated.h"

// 기본 스탯 초기화 이펙트
UCLASS()
class CATCHMEIFYOUCAN_API UGE_InitialCombatStats : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_InitialCombatStats();
};

// 무기 데미지 이펙트 (즉시 적용)
UCLASS()
class CATCHMEIFYOUCAN_API UGE_WeaponDamage : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_WeaponDamage();
};

// 무기 공격 쿨다운 이펙트
UCLASS(BlueprintType)
class CATCHMEIFYOUCAN_API UGE_WeaponAttackCooldown : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_WeaponAttackCooldown();
};

// 트랩 배치 쿨다운 이펙트
UCLASS(BlueprintType)
class CATCHMEIFYOUCAN_API UGE_TrapPlaceCooldown : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_TrapPlaceCooldown();
};

// 슬로우 트랩 이펙트 (이동속도 50% 감소)
UCLASS()
class CATCHMEIFYOUCAN_API UGE_SlowTrap : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_SlowTrap();
};

// 프리즈 트랩 이펙트 (완전 정지)
UCLASS()
class CATCHMEIFYOUCAN_API UGE_ImmobilizeTrap : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_ImmobilizeTrap();
};

UCLASS()
class CATCHMEIFYOUCAN_API UGE_DamageTrap : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_DamageTrap();
};