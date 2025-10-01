#include "CYConsumableBase.h"
#include "Character/CYPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "CYHealPotion.h"
#include "CYInvisibilityPotion.h"
#include "CYSpeedBoost.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"
#include "Kismet/GameplayStatics.h"

ACYConsumableBase::ACYConsumableBase()
{
    ItemName = FText::FromString("Base Consumable");
    ItemType = EItemType::Consumable;
    MaxStackCount = 10; // 소비 아이템은 스택 가능
}

bool ACYConsumableBase::UseItem(ACYPlayerCharacter* Character)
{
    if (!Character || !HasAuthority()) return false;

    UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
    if (!ASC) return false;

    bool bSuccess = false;
    
    for (TSubclassOf<UGameplayEffect> EffectClass : ConsumableEffects)
    {
        if (!EffectClass) continue;

        FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
        EffectContext.AddInstigator(Character, Character);
        
        FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
        if (EffectSpec.IsValid())
        {
            // Duration 오버라이드 (0보다 클 때만)
            if (OverrideDuration > 0.0f)
            {
                EffectSpec.Data->SetDuration(OverrideDuration, true);
            }
            
            // Primary 값 오버라이드
            if (OverridePrimaryValue > 0.0f)
            {
                if (EffectClass->IsChildOf(UGE_Heal::StaticClass()))
                {
                    EffectSpec.Data->SetSetByCallerMagnitude(FName("HealAmount"), OverridePrimaryValue);
                }
                else if (EffectClass->IsChildOf(UGE_SpeedBoost::StaticClass()))
                {
                    EffectSpec.Data->SetSetByCallerMagnitude(FName("SpeedBoostAmount"), OverridePrimaryValue);
                }
            }
            else  // 오버라이드 없으면 클래스 기본값 사용
            {
                if (EffectClass->IsChildOf(UGE_Heal::StaticClass()))
                {
                    if (ACYHealPotion* HealPotion = Cast<ACYHealPotion>(this))
                    {
                        EffectSpec.Data->SetSetByCallerMagnitude(FName("HealAmount"), HealPotion->HealAmount);
                    }
                }
                else if (EffectClass->IsChildOf(UGE_SpeedBoost::StaticClass()))
                {
                    if (ACYSpeedBoost* SpeedBoost = Cast<ACYSpeedBoost>(this))
                    {
                        EffectSpec.Data->SetSetByCallerMagnitude(FName("SpeedBoostAmount"), SpeedBoost->SpeedBoostAmount);
                    }
                }
            }

            ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
            bSuccess = true;
        }
    }

    if (bSuccess)
    {
        OnConsumableUsed(Character);
        UE_LOG(LogTemp, Warning, TEXT("%s used consumable: %s (Primary:%.1f, Duration:%.1f)"), 
               *Character->GetName(), *ItemName.ToString(), 
               OverridePrimaryValue, OverrideDuration);  // 디버그 로그 추가
    }
    
    return bSuccess;
}