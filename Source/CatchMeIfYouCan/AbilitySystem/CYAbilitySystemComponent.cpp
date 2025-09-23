#include "CYAbilitySystemComponent.h"

#include "Abilities/CYGameplayAbility.h"

UCYAbilitySystemComponent::UCYAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UCYAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);
	
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	// Avatar가 처음 할당되었을 경우, Avatar가 변경되었을 경우
	if (bHasNewPawnAvatar)
	{
		// TODO : 커스텀 AnimInstance의 FGameplayTagBlueprintPropertyMap타입 변수 초기화 진행
	}
}

void UCYAbilitySystemComponent::AbilityInputTagStarted(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputStartedSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UCYAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UCYAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UCYAbilitySystemComponent::AbilitySpecInputStarted(FGameplayAbilitySpec& Spec)
{
	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
		
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::GameCustom1, Spec.Handle, OriginalPredictionKey);
	}
}

void UCYAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
		
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void UCYAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
		
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

void UCYAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// 이번 프레임에 활성화할 어빌리티들을 저장할 정적 배열
	// 정적 변수로 선언하여 매 프레임마다 메모리 할당을 피함
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UCYGameplayAbility* CYAbilityCDO = Cast<UCYGameplayAbility>(AbilitySpec->Ability);
				// 지속 입력으로 활성화되는 어빌리티들 처리 (WhileInputActive 정책)
				if (CYAbilityCDO && CYAbilityCDO->GetActivationPolicy() == ECYAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}
	
	// 입력 시작 이벤트 처리 (Started 이벤트 → GameCustom1 이벤트 발생)
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputStartedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				// 활성화된 어빌리티만 Started 이벤트를 전달
				if (AbilitySpec->IsActive())
				{
					// GameCustom1 Replicated 이벤트 발생 
					AbilitySpecInputStarted(*AbilitySpec);
				}
			}
		}
	}
	
	// 입력 pressed 이벤트 처리 (Pressed 이벤트 → 활성화 vs 입력 이벤트 분기)
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				// Ability Spec의 InputPressed 플래그 설정
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive()) // 현재 활성화된 어빌리티인 경우
				{
					// 입력 이벤트만 전달
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else // 어빌리티가 비활성화된 경우
				{
					// 어빌리티가 비활성화 된 경우에만 새로 활성화 시도
					const UCYGameplayAbility* CYAbilityCDO = Cast<UCYGameplayAbility>(AbilitySpec->Ability);

					// OnInputTriggered Policy 어빌리티만 입력으로 활성화 가능
					if (CYAbilityCDO && CYAbilityCDO->GetActivationPolicy() == ECYAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	// 큐잉된 모든 어빌리티들 일괄 활성화
	// Held와 Pressed 입력을 한 번에 처리하여 중복 활성화 방지
	// Held 입력이 어빌리티를 활성화한 후 Pressed 입력이 같은 어빌리티에 입력 이벤트를 보내는 것을 방지
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		// 실제 어빌리티 활성화 시도
		// 여기서 CanActivateAbility 체크, Activation Group 관리 등이 수행됨
		TryActivateAbility(AbilitySpecHandle);
	}

	// Release 입력 이벤트 처리
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				// Ability Spec의 InputPressed 플래그 해제
				AbilitySpec->InputPressed = false;

				// 활성화된 어빌리티만 Release 이벤트를 전달
				if (AbilitySpec->IsActive())
				{
					// InputReleased Replicated 이벤트 발생 
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}
	
	// 캐시된 입력들 초기화
	// InputHeldSpecHandles는 입력이 Released될 때까지 유지
	InputStartedSpecHandles.Reset();
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}
