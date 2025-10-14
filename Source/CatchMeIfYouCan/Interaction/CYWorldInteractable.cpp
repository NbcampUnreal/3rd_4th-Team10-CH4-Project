#include "CYWorldInteractable.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CYLogChannels.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "Character/CYPlayerCharacter.h"
#include "Net/UnrealNetwork.h"

ACYWorldInteractable::ACYWorldInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void ACYWorldInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bWasConsumed);
}

void ACYWorldInteractable::OnInteractActiveStarted(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	if (HasAuthority())
	{
		CachedInteractors.Add(Interactor);
	}

	K2_OnInteractActiveStarted(Interactor);
}

void ACYWorldInteractable::OnInteractActiveEnded(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	if (HasAuthority())
	{
		CachedInteractors.RemoveSingleSwap(Interactor);
	}

	K2_OnInteractActiveEnded(Interactor);
}

void ACYWorldInteractable::OnInteractionSuccess(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	if (HasAuthority())
	{
		// 일회성 상호작용 객체인 경우 소모 처리 및 다른 상호작용자들의 상호작용 취소
		if (bShouldConsume)
		{
			bWasConsumed = true;

			TArray<TWeakObjectPtr<AActor>> TargetInteractors = MoveTemp(CachedInteractors);

			// 현재 상호작용 중인 다른 플레이어들의 상호작용을 취소
			for (TWeakObjectPtr<AActor>& TargetInteractor : TargetInteractors)
			{
				if (ACYPlayerCharacter* TargetCharacter = Cast<ACYPlayerCharacter>(TargetInteractor.Get()))
				{
					// 성공한 상호작용자는 제외
					if (Interactor == TargetCharacter)
					{
						continue;
					}

					// 다른 플레이어의 상호작용 어빌리티 취소
					if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetCharacter))
					{
						FGameplayTagContainer CancelAbilitiesTag;
						CancelAbilitiesTag.AddTag(CYGameplayTags::Ability_Action_AbilityInteract_Active);
						ASC->CancelAbilities(&CancelAbilitiesTag);
					}
				}
			}
		}
		else
		{
			// 반복 사용 가능한 객체의 경우 캐시만 정리
			CachedInteractors.Empty();
		}
	}
	
	K2_OnInteractionSuccess(Interactor);
}

bool ACYWorldInteractable::CanInteraction(const FCYInteractionQuery& InteractionQuery) const
{
	return bShouldConsume ? (bWasConsumed == false) : true;
}

void ACYWorldInteractable::OnRep_WasConsumed()
{
	// 기본 구현 (필요 시)
	UE_LOG(LogCY, Log, TEXT("bWasConsumed changed: %d"), bWasConsumed);
}

