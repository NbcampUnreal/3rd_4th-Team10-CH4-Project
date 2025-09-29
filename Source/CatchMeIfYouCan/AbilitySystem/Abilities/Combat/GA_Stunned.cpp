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
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ECYAbilityActivationPolicy::OnSpawn;
	
	// 태그 설정
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(CYGameplayTags::Ability_Stunned);
	SetAssetTags(AssetTags);

	FGameplayTagContainer OwnedTags;
	OwnedTags.AddTag(CYGameplayTags::State_Stunned);
	ActivationOwnedTags = OwnedTags;
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

	// 클라이언트에서는 활성화 불가
	if (!ActorInfo->IsNetAuthority())
	{
		return false;
	}

	// 서버: Stunned 태그 체크
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC || !ASC->HasMatchingGameplayTag(CYGameplayTags::State_Stunned))
	{
		return false;
	}

	return true;
}

void UGA_Stunned::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("=== GA_Stunned::ActivateAbility START (Authority: %s) ==="),
		ActorInfo->IsNetAuthority() ? TEXT("YES") : TEXT("NO"));

	// 서버만 실행
	if (!ActorInfo->IsNetAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to activate stunned, rejecting"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 정보 캐시
	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("No character found"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 움직임 완전 정지
	if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->DisableMovement();
		UE_LOG(LogTemp, Warning, TEXT("Movement disabled"));
	}

	// AnimInstance 캐시
	if (Character->GetMesh())
	{
		CachedAnimInstance = Character->GetMesh()->GetAnimInstance();
	}

	if (!CachedAnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("No AnimInstance found"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 회복 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(
		RecoveryTimerHandle,
		this,
		&UGA_Stunned::RecoverFromStun,
		StunnedDuration,
		false
	);
	UE_LOG(LogTemp, Warning, TEXT("Recovery timer set for %.1f seconds"), StunnedDuration);

	// 쓰러지는 애니메이션 재생
	if (FallingMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting falling montage"));

		// AbilityTask 사용
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
			
			UE_LOG(LogTemp, Warning, TEXT("Falling montage task activated"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No falling montage, going straight to lying"));
		OnFallingMontageCompleted();
	}

	UE_LOG(LogTemp, Warning, TEXT("%s is stunned for %.1f seconds"), 
		*Character->GetName(), StunnedDuration);
}

void UGA_Stunned::OnFallingMontageCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("Falling animation completed, starting lying animation"));

	// 쓰러져있는 애니메이션 루프 재생
	if (LyingMontage && CachedAnimInstance)
	{
		// 루프 재생
		CachedAnimInstance->Montage_Play(LyingMontage, 1.0f);
		
		UE_LOG(LogTemp, Warning, TEXT("Lying animation started (looping)"));
	}
}

void UGA_Stunned::OnFallingMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("Falling animation cancelled"));
	// 취소되어도 Lying 애니메이션은 재생
	OnFallingMontageCompleted();
}

void UGA_Stunned::RecoverFromStun()
{
	UE_LOG(LogTemp, Warning, TEXT("Recovering from stun"));

	// GameplayEffect로 HP 회복
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEffectSpecHandle HealSpec = MakeOutgoingGameplayEffectSpec(
			UGE_Heal::StaticClass(), 
			1  // Level
		);
		
		if (HealSpec.IsValid())
		{
			// 회복량 설정 (기본 1, BP에서 RecoveryHealth 수정 가능)
			HealSpec.Data->SetSetByCallerMagnitude(FName("HealAmount"), RecoveryHealth);
			
			// Self에게 적용
			ASC->ApplyGameplayEffectSpecToSelf(*HealSpec.Data.Get());
			
			UE_LOG(LogTemp, Warning, TEXT("Recovered +%.1f HP from stun"), RecoveryHealth);
		}
	}

	// Ability 종료
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UGA_Stunned::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("=== GA_Stunned::EndAbility (Cancelled: %s) ==="), 
		bWasCancelled ? TEXT("YES") : TEXT("NO"));

	// 타이머 정리
	if (GetWorld() && RecoveryTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("Recovery timer cleared"));
	}

	// 애니메이션 중지
	if (CachedAnimInstance)
	{
		if (FallingMontage && CachedAnimInstance->Montage_IsPlaying(FallingMontage))
		{
			CachedAnimInstance->Montage_Stop(0.2f, FallingMontage);
			UE_LOG(LogTemp, Warning, TEXT("Falling montage stopped"));
		}
		if (LyingMontage && CachedAnimInstance->Montage_IsPlaying(LyingMontage))
		{
			CachedAnimInstance->Montage_Stop(0.2f, LyingMontage);
			UE_LOG(LogTemp, Warning, TEXT("Lying montage stopped"));
		}
	}

	// 움직임 복구
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->SetMovementMode(MOVE_Walking);
			UE_LOG(LogTemp, Warning, TEXT("Movement restored"));
		}
	}

	// 캐시 정리
	CachedAnimInstance = nullptr;
	FallingMontageTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	UE_LOG(LogTemp, Warning, TEXT("Stunned ability fully ended"));
}