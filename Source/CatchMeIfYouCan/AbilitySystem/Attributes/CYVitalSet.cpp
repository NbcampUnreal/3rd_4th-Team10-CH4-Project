#include "CYVitalSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"
#include "Net/UnrealNetwork.h"

UCYVitalSet::UCYVitalSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
	
}

void UCYVitalSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Health가 MaxHealth를 초과하지 않도록 제한
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UCYVitalSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Health 속성이 변경되었을 때 처리
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		HandleHealthChange(Data);
	}
}

void UCYVitalSet::HandleHealthChange(const FGameplayEffectModCallbackData& Data)
{
	float NewHealth = GetHealth();
    
	// Health를 0과 MaxHealth 사이로 제한
	SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

	AActor* Owner = GetOwningActor();
	if (!Owner) return;

	// 서버인 경우만 실행
	if (!Owner->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	// 사망 처리 (체력이 0 이하)
	if (GetHealth() <= 0.0f)
	{
		if (ASC->HasMatchingGameplayTag(CYGameplayTags::State_Stunned))
		{
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("[Server] %s has died"), *Owner->GetName());

		// Stunned Ability 활성화
		if (UCYAbilitySystemComponent* CYASC = Cast<UCYAbilitySystemComponent>(ASC))
		{
			CYASC->TryActivateAbilityByTag(CYGameplayTags::Ability_Stunned);
		}
	}
	else if (WasDamaged(Data))
	{
		if (ASC->HasMatchingGameplayTag(CYGameplayTags::State_Stunned) ||
			ASC->HasMatchingGameplayTag(CYGameplayTags::State_Captured) ||
			ASC->HasMatchingGameplayTag(CYGameplayTags::State_Jail))
		{
			return;
		}
		
		FGameplayEventData EventData;
		EventData.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
		EventData.Target = Owner;
		EventData.ContextHandle = Data.EffectSpec.GetEffectContext();

		ASC->HandleGameplayEvent(CYGameplayTags::GameplayEvent_HitReact, &EventData);
	}
}

bool UCYVitalSet::WasDamaged(const FGameplayEffectModCallbackData& Data) const
{
	return Data.EvaluatedData.Magnitude < 0.0f;
}

void UCYVitalSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCYVitalSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCYVitalSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UCYVitalSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	if (GetOwningAbilitySystemComponent())
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCYVitalSet, Health, OldHealth);
        
		// 클라이언트에서도 Health 변경 처리 (UI 업데이트 등)
		UE_LOG(LogTemp, Log, TEXT("[Client] Health replicated: %.1f/%.1f"), 
			   GetHealth(), GetMaxHealth());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_Health: No valid AbilitySystemComponent"));
	}
}

void UCYVitalSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	if (GetOwningAbilitySystemComponent())
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCYVitalSet, MaxHealth, OldMaxHealth);
        
		// MaxHealth 변경 시 현재 Health도 재검증
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_MaxHealth: No valid AbilitySystemComponent"));
	}
}