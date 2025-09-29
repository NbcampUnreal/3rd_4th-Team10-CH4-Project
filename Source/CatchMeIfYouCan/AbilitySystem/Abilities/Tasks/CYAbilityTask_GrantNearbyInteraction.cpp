// Fill out your copyright notice in the Description page of Project Settings.


#include "CYAbilityTask_GrantNearbyInteraction.h"

#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "Interaction/CYInteractable.h"
#include "Physics/CYCollisionChannels.h"

UCYAbilityTask_GrantNearbyInteraction* UCYAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractables(UGameplayAbility* OwningAbility, float InteractionAbilityScanRange, float InteractionAbilityScanRate)
{
	UCYAbilityTask_GrantNearbyInteraction* Task = NewAbilityTask<UCYAbilityTask_GrantNearbyInteraction>(OwningAbility);
	Task->InteractionAbilityScanRange = InteractionAbilityScanRange;
	Task->InteractionAbilityScanRate = InteractionAbilityScanRate;
	return Task;
}

void UCYAbilityTask_GrantNearbyInteraction::Activate()
{
	Super::Activate();
	
	SetWaitingOnAvatar();

	// 주변 상호작용 가능한 객체 탐색 타이머 설정
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(QueryTimerHandle, this, &UCYAbilityTask_GrantNearbyInteraction::QueryInteractables, InteractionAbilityScanRate, true);
	}
}

void UCYAbilityTask_GrantNearbyInteraction::OnDestroy(bool bInOwnerFinished)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(QueryTimerHandle);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UCYAbilityTask_GrantNearbyInteraction::QueryInteractables()
{
	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActor();
	
	if (World && AvatarActor)
	{
		TSet<FObjectKey> RemoveKeys;
		GrantedInteractionAbilities.GetKeys(RemoveKeys);

		// "Stat Scenecquery" 같은 콘솔 명령어로 해당 Task의 성능 부하 확인 가능
		FCollisionQueryParams Params(SCENE_QUERY_STAT(UCYAbilityTask_GrantNearbyInteraction), false);

		TArray<FOverlapResult> OverlapResults;
		World->OverlapMultiByChannel(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, CY_TraceChannel_Interaction, FCollisionShape::MakeSphere(InteractionAbilityScanRange), Params);
		
		if (OverlapResults.Num() > 0)
		{
			// 감지된 객체들을 상호작용 가능한 객체로 필터링
			TArray<TScriptInterface<ICYInteractable>> Interactables;
			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				TScriptInterface<ICYInteractable> InteractableActor(OverlapResult.GetActor());
				if (InteractableActor)
				{
					Interactables.AddUnique(InteractableActor);
				}
		
				TScriptInterface<ICYInteractable> InteractableComponent(OverlapResult.GetComponent());
				if (InteractableComponent)
				{
					Interactables.AddUnique(InteractableComponent);
				}
			}

			// 각 상호작용 객체로부터 상호작용 정보 수집
			FCYInteractionQuery InteractionQuery;
			InteractionQuery.RequestingAvatar = AvatarActor;
			InteractionQuery.RequestingController = Cast<AController>(AvatarActor->GetOwner());
		
			TArray<FCYInteractionInfo> InteractionInfos;
			for (TScriptInterface<ICYInteractable>& Interactable : Interactables)
			{
				FCYInteractionInfoBuilder InteractionInfoBuilder(Interactable, InteractionInfos);
				Interactable->GatherPostInteractionInfos(InteractionQuery, InteractionInfoBuilder);
			}
		
			for (FCYInteractionInfo& InteractionInfo : InteractionInfos)
			{
				if (InteractionInfo.AbilityToGrant)
				{
					// 현재 감지된 객체가 주는 Ability를 이미 부여받았는지 확인
					FObjectKey ObjectKey(InteractionInfo.AbilityToGrant);
					if (GrantedInteractionAbilities.Find(ObjectKey))
					{
						RemoveKeys.Remove(ObjectKey);
					}
					// 새로 받는 Interaction 타입의 Ability인 경우 부여
					else
					{
						FGameplayAbilitySpec Spec(InteractionInfo.AbilityToGrant, 1, INDEX_NONE, this);
						FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(Spec);
						GrantedInteractionAbilities.Add(ObjectKey, SpecHandle);
					}
				}
			}
		}
		// 감지되지 않은 객체 타입의 Interaction Ability는 제거
		for (const FObjectKey& RemoveKey : RemoveKeys)
		{
			AbilitySystemComponent->ClearAbility(GrantedInteractionAbilities[RemoveKey]);
			GrantedInteractionAbilities.Remove(RemoveKey);
		}
	}
}
