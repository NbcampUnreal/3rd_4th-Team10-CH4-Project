#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"
#include "AbilitySystem/Attributes/CYCombatAttributeSet.h"
#include "AbilitySystem/Attributes/CYVitalSet.h"
#include "AbilitySystem/CYCombatGameplayTags.h"

UGE_InitialCombatStats::UGE_InitialCombatStats()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
    
	// 체력 초기화
	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UCYVitalSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.0f));
	Modifiers.Add(HealthModifier);
    
	// 최대 체력 초기화
	FGameplayModifierInfo MaxHealthModifier;
	MaxHealthModifier.Attribute = UCYVitalSet::GetMaxHealthAttribute();
	MaxHealthModifier.ModifierOp = EGameplayModOp::Additive;
	MaxHealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.0f));
	Modifiers.Add(MaxHealthModifier);
    
	// 기본 속도 400 유지
	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UCYCombatAttributeSet::GetMoveSpeedAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Additive;
	MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.0f));
	Modifiers.Add(MoveSpeedModifier);
    
	// 공격력 초기화
	FGameplayModifierInfo AttackPowerModifier;
	AttackPowerModifier.Attribute = UCYCombatAttributeSet::GetAttackPowerAttribute();
	AttackPowerModifier.ModifierOp = EGameplayModOp::Additive;
	AttackPowerModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(50.0f));
	Modifiers.Add(AttackPowerModifier);
    
	UE_LOG(LogTemp, Warning, TEXT("InitialCombatStats GE created"));
}

// 무기 데미지 이펙트
UGE_WeaponDamage::UGE_WeaponDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
    
	// Health 데미지
	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UCYVitalSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;
    
	// SetByCaller로 동적 데미지 설정
	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataName = FName("Damage");
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);
    
    
	Modifiers.Add(HealthModifier);
}


// 무기 공격 쿨다운
UGE_WeaponAttackCooldown::UGE_WeaponAttackCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.0f));
    
    UE_LOG(LogTemp, Warning, TEXT("WeaponAttackCooldown GE created"));
}

// 트랩 배치 쿨다운 이펙트
UGE_TrapPlaceCooldown::UGE_TrapPlaceCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.5f));
    
    UE_LOG(LogTemp, Warning, TEXT("TrapPlaceCooldown GE created"));
}

// 속도 400 -> 50
UGE_SlowTrap::UGE_SlowTrap()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));
    
	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UCYCombatAttributeSet::GetMoveSpeedAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Override;
	MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(50.0f));
	Modifiers.Add(MoveSpeedModifier);

	FGameplayTag CueTag = CYGameplayTags::GameplayCue_Trap_Slow;
	GameplayCues.Add(FGameplayEffectCue(CueTag, 0.0f, 0.0f));
    
    UE_LOG(LogTemp, Warning, TEXT("SlowTrap GE created: 400->50"));
}

// 속도 400 -> 0
UGE_ImmobilizeTrap::UGE_ImmobilizeTrap()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(3.0f));
    
	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UCYCombatAttributeSet::GetMoveSpeedAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Override;
	MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.0f));
	Modifiers.Add(MoveSpeedModifier);

	FGameplayTag CueTag = CYGameplayTags::GameplayCue_Trap_Freeze;
	GameplayCues.Add(FGameplayEffectCue(CueTag, 0.0f, 0.0f));
    
    UE_LOG(LogTemp, Warning, TEXT("ImmobilizeTrap GE created: 400->0"));
}

// 데미지 트랩 이펙트
UGE_DamageTrap::UGE_DamageTrap()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
    
	// Health 데미지
	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UCYVitalSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;
    
	// SetByCaller로 동적 데미지 설정 (트랩별로 다른 데미지 가능)
	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataName = FName("TrapDamage");
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);
    
	Modifiers.Add(HealthModifier);

	FGameplayTag CueTag = CYGameplayTags::GameplayCue_Trap_Damage;
	GameplayCues.Add(FGameplayEffectCue(CueTag, 0.0f, 0.0f));
    
	UE_LOG(LogTemp, Warning, TEXT("DamageTrap GE created"));
}

UGE_Heal::UGE_Heal()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
    
	// Health 회복
	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UCYVitalSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;
    
	// SetByCaller로 동적 회복량 설정
	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataName = FName("HealAmount");
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);
    
	Modifiers.Add(HealthModifier);

	// GameplayCue 추가
	FGameplayTag CueTag = CYGameplayTags::GameplayCue_Consumable_Heal;
	GameplayCues.Add(FGameplayEffectCue(CueTag, 0.0f, 0.0f));
    
	UE_LOG(LogTemp, Warning, TEXT("Heal GE created"));
}

// 속도 증가 이펙트
UGE_SpeedBoost::UGE_SpeedBoost()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(4.0f));
    
	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UCYCombatAttributeSet::GetMoveSpeedAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataName = FName("SpeedBoostAmount");
	MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);
	
	Modifiers.Add(MoveSpeedModifier);

	// GameplayCue 추가
	FGameplayTag CueTag = CYGameplayTags::GameplayCue_Consumable_SpeedBoost;
	GameplayCues.Add(FGameplayEffectCue(CueTag, 0.0f, 0.0f));
    
	UE_LOG(LogTemp, Warning, TEXT("SpeedBoost GE created"));
}

// 투명화 이펙트
UGE_Invisibility::UGE_Invisibility()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(4.0f));

	// GameplayCue 추가
	FGameplayTag CueTag = CYGameplayTags::GameplayCue_Consumable_Invisibility;
	GameplayCues.Add(FGameplayEffectCue(CueTag, 0.0f, 0.0f));
}