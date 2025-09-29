#include "CYVitalSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
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
		HandleHealthChange();
	}
}

void UCYVitalSet::HandleHealthChange()
{
	float NewHealth = GetHealth();
    
	// Health를 0과 MaxHealth 사이로 제한
	SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

	// 사망 처리
	if (GetHealth() <= 0.0f)
	{
		if (AActor* Owner = GetOwningActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has died (Health: %.1f)"), *Owner->GetName(), GetHealth());

			// State_Stunned 태그 부여
			if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
			{
				FGameplayTagContainer TagsToAdd;
				TagsToAdd.AddTag(CYGameplayTags::State_Stunned);
				ASC->AddLooseGameplayTags(TagsToAdd);
			}
            
			// 여기에 사망 이벤트 처리 추가 가능
		}
	}
	else
	{
		// 체력이 회복되면 Stunned 태그 제거
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			if (ASC->HasMatchingGameplayTag(CYGameplayTags::State_Stunned))
			{
				FGameplayTagContainer TagsToRemove;
				TagsToRemove.AddTag(CYGameplayTags::State_Stunned);
				ASC->RemoveLooseGameplayTags(TagsToRemove);
			}
		}
		
		// Health 변경 로그
		if (AActor* Owner = GetOwningActor())
		{
			UE_LOG(LogTemp, Log, TEXT("%s Health changed: %.1f/%.1f"), 
				   *Owner->GetName(), GetHealth(), GetMaxHealth());
		}
	}
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
        
		// 클라이언트에서도 Health 변경 처리
		HandleHealthChange();
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
		HandleHealthChange();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_MaxHealth: No valid AbilitySystemComponent"));
	}
}
