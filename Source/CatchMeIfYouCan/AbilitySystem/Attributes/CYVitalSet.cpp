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

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	// 사망 처리 (체력이 0 이하)
	if (GetHealth() <= 0.0f)
	{
		// 이미 Stunned 어빌리티가 활성화되어 있으면 중복 방지
		if (ASC->HasMatchingGameplayTag(CYGameplayTags::State_Stunned))
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Server] Already stunned, skipping"));
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("[Server] %s has died (Health: %.1f)"), 
			*Owner->GetName(), GetHealth());

		// Loose 태그로 Stunned 상태 표시 (어빌리티가 이 태그를 확인함)
		FGameplayTagContainer TagsToAdd;
		TagsToAdd.AddTag(CYGameplayTags::State_Stunned);
		ASC->AddLooseGameplayTags(TagsToAdd);

		UE_LOG(LogTemp, Warning, TEXT("[Server] Added Stunned tag to %s"), *Owner->GetName());

		// Stunned Ability 활성화 시도
		if (UCYAbilitySystemComponent* CYASC = Cast<UCYAbilitySystemComponent>(ASC))
		{
			bool bActivated = CYASC->TryActivateAbilityByTag(CYGameplayTags::Ability_Stunned);
			UE_LOG(LogTemp, Warning, TEXT("[Server] Stunned ability activation: %s"), 
				bActivated ? TEXT("SUCCESS") : TEXT("FAILED"));

			// 활성화 실패 시 태그 제거 (정리)
			if (!bActivated)
			{
				FGameplayTagContainer TagsToRemove;
				TagsToRemove.AddTag(CYGameplayTags::State_Stunned);
				ASC->RemoveLooseGameplayTags(TagsToRemove);
				UE_LOG(LogTemp, Error, TEXT("[Server] Failed to activate Stunned ability, removed tag"));
			}
		}
	}
	// 체력 회복 시 처리
	else if (GetHealth() > 0.0f)
	{
		// Stunned 어빌리티가 활성화되어 있으면 스킵 (어빌리티가 알아서 처리)
		// 어빌리티가 종료되면서 태그를 제거할 것
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