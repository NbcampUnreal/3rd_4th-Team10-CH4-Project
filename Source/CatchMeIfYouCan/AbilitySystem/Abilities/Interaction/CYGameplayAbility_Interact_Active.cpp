#include "CYGameplayAbility_Interact_Active.h"

#include "AbilitySystemComponent.h"
#include "CYGameplayAbility_Interact.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "AbilitySystem/Abilities/Tasks/CYAbilityTask_WaitForInvalidInteraction.h"
#include "Character/CYCharacterBase.h"
#include "Character/CYStatusGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/CYWorldInteractable.h"


UCYGameplayAbility_Interact_Active::UCYGameplayAbility_Interact_Active()
{
	ActivationPolicy = ECYAbilityActivationPolicy::Manual;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 클라이언트의 취소 요청을 서버가 존중 (입력 해제 시 즉시 취소)
	bServerRespectsRemoteAbilityCancellation = true;
	
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 서버에서만 종료 가능 (보안상 중요한 상호작용 보호)
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;
	
	AbilityTags.AddTag(CYGameplayTags::Ability_Action_AbilityInteract_Active);
	ActivationOwnedTags.AddTag(CYGameplayTags::Status_Action_AbilityInteract);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CYGameplayTags::Ability_Action_AbilityInteract_Active;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UCYGameplayAbility_Interact_Active::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (TriggerEventData == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 상호작용 대상 및 정보 초기화
	if (InitializeAbility(const_cast<AActor*>(TriggerEventData->Target.Get())) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 상호작용 대상에게 홀딩 시작 알림
	if (ACYWorldInteractable* WorldInteractable = Cast<ACYWorldInteractable>(InteractableActor))
	{
		WorldInteractable->OnInteractActiveStarted(GetCYCharacterFromActorInfo());
	}

	// 즉시 실행 vs 홀딩 분기 결정
	if (InteractionInfo.Duration <= 0.f)
	{
		TriggerInteraction();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 기존 움직임 입력 플러시 (홀딩 중 의도치 않은 움직임 방지)
	FlushPressedInput(MoveInputAction);
	if (ACYCharacterBase* CYCharacter = GetCYCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* CharacterMovement = CYCharacter->GetCharacterMovement())
		{
			CharacterMovement->StopMovementImmediately();
		}
	}
	
	// TODO : GameplayMessageSubsystem을 사용중이지 않기 때문에 주석 처리(대안 찾기 or 해당 시스템 도입 고려)
	// FCYInteractionMessage Message;
	// Message.Instigator = GetAvatarActorFromActorInfo();
	// Message.bShouldRefresh = true;
	// Message.bSwitchActive = true;
	// Message.InteractionInfo = InteractionInfo;
	// UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	// MessageSubsystem.BroadcastMessage(CYGameplayTags::Message_Interaction_Progress, Message);

	// 홀딩 시작 애니메이션 재생
	if (UAnimMontage* ActiveStartMontage = InteractionInfo.ActiveStartMontage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("InteractMontage"), ActiveStartMontage, 1.f, NAME_None, true, 1.f, 0.f, false))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}

	// 홀딩 취소 조건 모니터링 시작

	// 1. 위치/각도 이탈 감지 태스크
	if (UCYAbilityTask_WaitForInvalidInteraction* InvalidInteractionTask = UCYAbilityTask_WaitForInvalidInteraction::WaitForInvalidInteraction(this, AcceptanceAngle, AcceptanceDistance))
	{
		InvalidInteractionTask->OnInvalidInteraction.AddDynamic(this, &ThisClass::OnInvalidInteraction);
		InvalidInteractionTask->ReadyForActivation();
	}

	// 2. 입력 해제 감지 태스크
	if (UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false))
	{
		InputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		InputReleaseTask->ReadyForActivation();
	}

	// 홀딩 지속시간 타이머 시작
	// InteractionInfo.Duration 시간 후 OnDurationEnded() 호출
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::OnDurationEnded, InteractionInfo.Duration, false);
}

void UCYGameplayAbility_Interact_Active::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ACYCharacterBase* CYCharacter = GetCYCharacterFromActorInfo())
	{
		// 상호작용 대상에게 홀딩 종료 알림
		if (ACYWorldInteractable* WorldInteractable = Cast<ACYWorldInteractable>(InteractableActor))
		{
			WorldInteractable->OnInteractActiveEnded(CYCharacter);
		}

		// TODO : GameplayMessageSubsystem을 사용중이지 않기 때문에 주석 처리(대안 찾기 or 해당 시스템 도입 고려)
		// FCYInteractionMessage Message;
		// Message.Instigator = CYCharacter;
		// Message.bShouldRefresh = false;
		// Message.bSwitchActive = true;
		// UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		// MessageSubsystem.BroadcastMessage(CYGameplayTags::Message_Interaction_Notice, Message);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCYGameplayAbility_Interact_Active::OnInvalidInteraction()
{
	// 홀딩 즉시 취소 (위치/각도 이탈로 인한 취소)
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UCYGameplayAbility_Interact_Active::OnInputReleased(float TimeHeld)
{
	// 홀딩 즉시 취소 (플레이어가 키를 뗌)
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UCYGameplayAbility_Interact_Active::OnDurationEnded()
{
	// 멀티플레이어 환경에서 서버와 클라이언트 간 타이밍 동기화
	// 서버만 대기하고 클라이언트는 즉시 진행 (서버 권한 보장)
	if (UAbilityTask_NetworkSyncPoint* NetSyncTask = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait))
	{
		NetSyncTask->OnSync.AddDynamic(this, &ThisClass::OnNetSync);
		NetSyncTask->ReadyForActivation();
	}
}

void UCYGameplayAbility_Interact_Active::OnNetSync()
{
	// 실제 상호작용 트리거 시도
	if (TriggerInteraction())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

bool UCYGameplayAbility_Interact_Active::TriggerInteraction()
{
	bool bTriggerSuccessful = false;
	bool bCanActivate = false;

	// 상호작용 실행을 위한 게임플레이 이벤트 데이터 구성
	FGameplayEventData Payload;
	Payload.EventTag = CYGameplayTags::Ability_Action_AbilityInteract; // 상호작용 실행 태그
	Payload.Instigator = GetAvatarActorFromActorInfo(); // 상호작용 요청자 (플레이어)
	Payload.Target = InteractableActor;	// 상호작용 대상 액터

	// 상호작용 대상이 이벤트 데이터를 커스터마이징할 기회 제공
	Interactable->CustomizeInteractionEventData(CYGameplayTags::Ability_Action_AbilityInteract, Payload);
	
	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo())
	{
		if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromClass(InteractionInfo.AbilityToGrant))
		{
			bCanActivate = AbilitySpec->Ability->CanActivateAbility(AbilitySpec->Handle, AbilitySystem->AbilityActorInfo.Get());
			
			// 게임플레이 이벤트를 통해 실제 상호작용 어빌리티 트리거
			bTriggerSuccessful = AbilitySystem->TriggerAbilityFromGameplayEvent(
				AbilitySpec->Handle,
				AbilitySystem->AbilityActorInfo.Get(),
				CYGameplayTags::Ability_Action_AbilityInteract,  // 트리거 태그
				&Payload,
				*AbilitySystem
			);
		}
	}

	return bCanActivate || bTriggerSuccessful;
}