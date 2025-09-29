#include "CYVitalSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
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

	AActor* Owner = GetOwningActor();
	if (!Owner) return;

	// 서버인 경우만 실행
	if (!Owner->HasAuthority())
	{
		return;
	}

	// 사망 처리
	if (GetHealth() <= 0.0f)
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (!ASC) return;

		// 이미 Stunned 상태면 중복 처리 안 함
		if (ASC->HasMatchingGameplayTag(CYGameplayTags::State_Stunned))
		{
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("[Server] %s has died (Health: %.1f)"), 
			*Owner->GetName(), GetHealth());

		// State_Stunned 태그 부여
		FGameplayTagContainer TagsToAdd;
		TagsToAdd.AddTag(CYGameplayTags::State_Stunned);
		ASC->AddLooseGameplayTags(TagsToAdd);

		// Stunned Ability 활성화
		if (UCYAbilitySystemComponent* CYASC = Cast<UCYAbilitySystemComponent>(ASC))
		{
			bool bActivated = CYASC->TryActivateAbilityByTag(CYGameplayTags::Ability_Stunned);
			UE_LOG(LogTemp, Warning, TEXT("[Server] Stunned ability activation: %s"), 
				bActivated ? TEXT("SUCCESS") : TEXT("FAILED"));
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
				
				UE_LOG(LogTemp, Warning, TEXT("[Server] Stunned tag removed due to health recovery"));
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("[Server] %s Health changed: %.1f/%.1f"), 
			   *Owner->GetName(), GetHealth(), GetMaxHealth());
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
