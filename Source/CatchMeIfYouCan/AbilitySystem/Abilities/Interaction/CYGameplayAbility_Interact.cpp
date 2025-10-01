// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_GrantNearbyInteraction.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_WaitForInteractableTraceHit.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_WaitInputStart.h"
#include "Character/CYPlayerCharacter.h"
#include "Character/CYStatusGameplayTags.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Interaction/CYInteractionQuery.h"
#include "Physics/CYCollisionChannels.h"

UCYGameplayAbility_Interact::UCYGameplayAbility_Interact()
{
	ActivationPolicy = ECYAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCYGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FCYInteractionQuery InteractionQuery;
	InteractionQuery.RequestingAvatar = GetAvatarActorFromActorInfo();
	InteractionQuery.RequestingController = GetControllerFromActorInfo();

	// 1. 시선 방향 정밀 타겟팅 태스크 시작
	if (UCYAbilityTask_WaitForInteractableTraceHit* TraceHitTask = UCYAbilityTask_WaitForInteractableTraceHit::WaitForInteractableTraceHit(this, InteractionQuery, CY_TraceChannel_Interaction, MakeTargetLocationInfoFromOwnerActor(), InteractionTraceRange, InteractionTraceRate, bShowTraceDebug))
	{
		// 상호작용 가능한 객체가 변경될 때마다 UI 업데이트
		TraceHitTask->InteractableChanged.AddDynamic(this, &ThisClass::UpdateInteractions);
		TraceHitTask->ReadyForActivation();
	}

	// 2. 서버에서만 주변 객체 스캔 및 어빌리티 부여 태스크 시작
	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		// 주변 상호작용 가능한 객체들 타입에 대한 어빌리티 부여
		UCYAbilityTask_GrantNearbyInteraction* GrantAbilityTask = UCYAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractables(this, InteractionScanRange, InteractionScanRate);
		GrantAbilityTask->ReadyForActivation();
	}

	// 3. 상호작용 입력 대기 시작
	WaitInputStart();
}

void UCYGameplayAbility_Interact::UpdateInteractions(const TArray<FCYInteractionInfo>& InteractionInfos)
{
	// TODO : Lyra의 GameplayMessageSubsystem을 사용중이지 않기 때문에 주석 처리(대안 찾기 or 해당 시스템 도입 고려)
	// FCYInteractionMessage Message;
	// Message.Instigator = GetAvatarActorFromActorInfo();
	// Message.bShouldRefresh = true;
	// Message.bSwitchActive = (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(CYGameplayTags::Status_Action_AbilityInteract) == false);
	// Message.InteractionInfo = InteractionInfos.Num() > 0 ? InteractionInfos[0] : FCYInteractionInfo();
	//
	// UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetAvatarActorFromActorInfo());
	// MessageSystem.BroadcastMessage(CYGameplayTags::Message_Interaction_Notice, Message);

	CurrentInteractionInfos = InteractionInfos;
}

void UCYGameplayAbility_Interact::TriggerInteraction()
{
	if (CurrentInteractionInfos.Num() == 0)
	{
		return;
	}
	
	ACYPlayerCharacter* CYPlayerCharacter = Cast<ACYPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (CYPlayerCharacter && CYPlayerCharacter->GetMovementComponent()->IsFalling())
	{
		return;
	}
	
	if (GetAbilitySystemComponentFromActorInfo())
	{
		const FCYInteractionInfo& InteractionInfo = CurrentInteractionInfos[0];

		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableActor = nullptr;

		if (UObject* Object = InteractionInfo.Interactable.GetObject())
		{
			if (AActor* Actor = Cast<AActor>(Object))
			{
				InteractableActor = Actor;
			}
			else if (UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
			{
				InteractableActor = ActorComponent->GetOwner();
			}
		}

		// AbilityInteract_Active 어빌리티를 EventData(상호작용 대상자, 상호작용되는 물체)정보를 넘겨서 trigger 되도록 함. 
		FGameplayEventData Payload;
		Payload.EventTag = CYGameplayTags::Ability_Action_AbilityInteract_Active;
		Payload.Instigator = Instigator;
		Payload.Target = InteractableActor;
		
		SendGameplayEvent(CYGameplayTags::Ability_Action_AbilityInteract_Active, Payload);
	}
}

void UCYGameplayAbility_Interact::WaitInputStart()
{
	if (UCYAbilityTask_WaitInputStart* InputStartTask = UCYAbilityTask_WaitInputStart::WaitInputStart(this))
	{
		InputStartTask->OnStart.AddDynamic(this, &ThisClass::OnInputStart);
		InputStartTask->ReadyForActivation();
	}
}

void UCYGameplayAbility_Interact::OnInputStart()
{
	TriggerInteraction();
	WaitInputStart();
}