// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Jump.h"

#include "CYLogChannels.h"
#include "Character/CYCharacterBase.h"

UCYGameplayAbility_Jump::UCYGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ECYAbilityActivationPolicy::OnInputTriggered;
}

bool UCYGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// ActorInfo에서 CYCharacter 가져오기
	const ACYCharacterBase* CYCharacter = GetCYCharacterFromActorInfo();
	if (!CYCharacter)
	{
		return false;
	}

	// Character의 CanJump() 체크
	return CYCharacter->CanJump();
}

void UCYGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Prediction Key 체크
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}

	// Ability Cost와 Cooldown 커밋
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 점프 시작
	StartJump();
    
	UE_LOG(LogCY, Warning, TEXT("[%s] Jump Ability Activated"), 
		HasAuthority(&ActivationInfo) ? TEXT("Server") : TEXT("Client"));
}

void UCYGameplayAbility_Jump::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UCYGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	StopJump();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCYGameplayAbility_Jump::StartJump()
{
	ACYCharacterBase* CYCharacter = GetCYCharacterFromActorInfo();
	if (!CYCharacter)
	{
		return;
	}

	// 웅크리기 해제 후 점프
	CYCharacter->UnCrouch();
	CYCharacter->Jump();

}

void UCYGameplayAbility_Jump::StopJump()
{
	ACYCharacterBase* CYCharacter = GetCYCharacterFromActorInfo();
	if (!CYCharacter)
	{
		return;
	}

	// 점프 중지 (점프 높이 조절)
	CYCharacter->StopJumping();
    
	UE_LOG(LogCY, Warning, TEXT("Character Jump Stopped"));
}

