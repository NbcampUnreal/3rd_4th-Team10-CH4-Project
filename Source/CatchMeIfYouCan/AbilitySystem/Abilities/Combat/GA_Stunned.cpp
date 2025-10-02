#include "AbilitySystem/Abilities/Combat/GA_Stunned.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "AbilitySystem/Effects/CYCombatGameplayEffects.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

UGA_Stunned::UGA_Stunned()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	ActivationPolicy = ECYAbilityActivationPolicy::OnInputTriggered;
	
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(CYGameplayTags::Ability_Stunned);
	SetAssetTags(AssetTags);
	
	FGameplayTagContainer BlockedTags;
	BlockedTags.AddTag(CYGameplayTags::State_Combat_Attacking);
	BlockedTags.AddTag(CYGameplayTags::Ability_Combat_WeaponAttack);
	BlockedTags.AddTag(CYGameplayTags::Ability_Combat_PlaceTrap);
	ActivationBlockedTags = BlockedTags;
}

bool UGA_Stunned::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo->IsNetAuthority())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return false;
	}
	bool bHasTag = ASC->HasMatchingGameplayTag(CYGameplayTags::State_Stunned);
    
	return !bHasTag;
}

void UGA_Stunned::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("=== GA_Stunned::ActivateAbility (Authority: %s) ==="),
		ActorInfo->IsNetAuthority() ? TEXT("Server") : TEXT("Client"));

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 서버: 움직임 정지 + 타이머
	if (ActorInfo->IsNetAuthority())
	{
		// Stunned 상태 GE 적용
		if (StunnedStateGEClass)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			if (ASC)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StunnedStateGEClass, 1.0f, EffectContext);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					UE_LOG(LogTemp, Warning, TEXT("Applied Stunned GE"));
				}
			}
		}

		// 스턴 중에 중력은 작용하고 이동만 막기
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->StopMovementImmediately();
			MovementComp->SetMovementMode(MOVE_Walking);
			MovementComp->MaxWalkSpeed = 0.0f;
			MovementComp->MaxAcceleration = 0.0f;
			Character->ForceNetUpdate();
		}

		GetWorld()->GetTimerManager().SetTimer(
			RecoveryTimerHandle,
			this,
			&UGA_Stunned::RecoverFromStun,
			StunnedDuration,
			false
		);
		UE_LOG(LogTemp, Warning, TEXT("Movement disabled, timer set for %.1fs"), StunnedDuration);
	}

	// AnimInstance 캐시 (서버 + 클라이언트)
	if (Character->GetMesh())
	{
		CachedAnimInstance = Character->GetMesh()->GetAnimInstance();
	}

	if (!CachedAnimInstance)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Falling 몽타주 재생
	if (FallingMontage)
	{
		// AbilityTask 사용 - 자동 네트워크 동기화
		FallingMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("PlayFallingMontage"),
			FallingMontage,
			1.0f
		);

		if (FallingMontageTask)
		{
			FallingMontageTask->OnCompleted.AddDynamic(this, &UGA_Stunned::OnFallingMontageCompleted);
			FallingMontageTask->OnCancelled.AddDynamic(this, &UGA_Stunned::OnFallingMontageCancelled);
			FallingMontageTask->OnInterrupted.AddDynamic(this, &UGA_Stunned::OnFallingMontageCancelled);
			FallingMontageTask->ReadyForActivation();
			
			UE_LOG(LogTemp, Warning, TEXT("Falling montage started"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Falling montage configured!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_Stunned::OnFallingMontageCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("Falling animation completed - just waiting for timer"));
	// 몽타주가 끝나도 타이머가 끝날 때까지 대기
}

void UGA_Stunned::OnFallingMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("Falling animation cancelled"));
	// 그냥 무시하고 타이머 기다림
}

void UGA_Stunned::RecoverFromStun()
{
	if (!CachedActorInfo || !CachedActorInfo->IsNetAuthority())
	{
		return;
	}

	if (!CachedActorInfo->OwnerActor.IsValid() || 
		!CachedActorInfo->AvatarActor.IsValid() ||
		!CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("RecoverFromStun: Cached actors are no longer valid, aborting recovery"));
		// 어빌리티 종료 시도
		if (GetWorld() && RecoveryTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Recovering from stun"));

	// HP 회복
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEffectSpecHandle HealSpec = MakeOutgoingGameplayEffectSpec(UGE_Heal::StaticClass(), 1);
		
		if (HealSpec.IsValid())
		{
			HealSpec.Data->SetSetByCallerMagnitude(FName("HealAmount"), RecoveryHealth);
			ASC->ApplyGameplayEffectSpecToSelf(*HealSpec.Data.Get());
			UE_LOG(LogTemp, Warning, TEXT("Recovered +%.1f HP"), RecoveryHealth);
		}
	}

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UGA_Stunned::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("=== GA_Stunned::EndAbility ==="));

	// 타이머 정리
	if (GetWorld() && RecoveryTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
	}

	// 애니메이션 정리 (서버 + 클라이언트)
	if (CachedAnimInstance && FallingMontage)
	{
		if (CachedAnimInstance->Montage_IsPlaying(FallingMontage))
		{
			CachedAnimInstance->Montage_Stop(0.2f, FallingMontage);
			UE_LOG(LogTemp, Warning, TEXT("Stunned montage stopped"));
		}
	}

	// 움직임 복구 + 이펙트 제거 (서버만)
	if (ActorInfo->IsNetAuthority())
	{
		ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (Character)
		{
			if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed = 400.0f;
				MovementComp->MaxAcceleration = 2048.0f;
				Character->ForceNetUpdate();
			}

			// Stunned GE 제거 (태그로 쿼리)
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
			{
				FGameplayEffectQuery Query;
				Query.EffectTagQuery = FGameplayTagQuery::MakeQuery_MatchTag(CYGameplayTags::State_Stunned);
				ASC->RemoveActiveEffects(Query);
            
				UE_LOG(LogTemp, Warning, TEXT("Stunned GE removed by tag"));
			}
		}
	}

	// 캐시 정리
	CachedAnimInstance = nullptr;
	FallingMontageTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}