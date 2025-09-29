// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact_Arrest.h"

#include "EngineUtils.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "Character/CYCharacterBase.h"
#include "GameModes/InGame/CYInGameState.h"
#include "Player/CYPlayerState.h"

UCYGameplayAbility_Interact_Arrest::UCYGameplayAbility_Interact_Arrest()
{
}

void UCYGameplayAbility_Interact_Arrest::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TriggerEventData)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 서버에서만 실제 체포 가능
	// 클라에서는 애니/이펙트만 재생 가능
	if (!HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	ACYCharacterBase* InstigatorCop = GetCYCharacterFromActorInfo();
	ACYCharacterBase* TargetRobber = Cast<ACYCharacterBase>(InteractableActor);
	if (!InstigatorCop || !TargetRobber)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 서버 검증(팀/스턴/거리/중복 체포 등)
	if (!ValidateArrest(InstigatorCop, TargetRobber))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}
	
	// 체포 실행
	DoArrest(InstigatorCop, TargetRobber);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UCYGameplayAbility_Interact_Arrest::ValidateArrest(const ACYCharacterBase* InstigatorCop, const ACYCharacterBase* TargetRobber) const
{
	if (!InstigatorCop || !TargetRobber)
	{
		return false;
	}
	
	// 팀/역할 확인 TODO : GameplayTag로 팀 체크 고려
	const ACYPlayerState* CopPS = InstigatorCop->GetPlayerState<ACYPlayerState>();
	const ACYPlayerState* RobPS = TargetRobber->GetPlayerState<ACYPlayerState>();
	if (!CopPS || !RobPS)
	{
		return false;
	}
	if (CopPS->GetTeamRole() != ECYTeamRole::Cop)
	{
		return false;
	}
	if (RobPS->GetTeamRole() != ECYTeamRole::Robber)
	{
		return false;
	}
	
	// 거리 검증
	const float DistSquare = FVector::DistSquared(InstigatorCop->GetActorLocation(), TargetRobber->GetActorLocation());
	if (DistSquare > FMath::Square(MaxArrestDistance))
	{
		return false;
	}

	// 이미 감옥 상태면 중복 방지
	if (TargetRobber->HasGameplayTag(CYGameplayTags::State_Jail))
	{
		return false;
	}

	// 스턴 요구
	if (bFailIfNotStunned && !TargetRobber->HasGameplayTag(CYGameplayTags::State_Stunned))
	{
		return false;
	}

	return true;
}

void UCYGameplayAbility_Interact_Arrest::DoArrest(ACYCharacterBase* InstigatorCop, ACYCharacterBase* TargetRobber)
{
	if (!InstigatorCop || !TargetRobber)
	{
		return;
	}

	// 태그 갱신: 스턴 제거, 캡쳐/감옥 부여
	TargetRobber->RemoveGameplayTag(CYGameplayTags::State_Stunned);
	TargetRobber->AddGameplayTag(CYGameplayTags::State_Jail);

	// 감방 위치 탐색 → 텔레포트
	FTransform JailTransform;
	if (FindJailTransform(JailTransform))
	{
		TargetRobber->TeleportTo(JailTransform.GetLocation(), JailTransform.GetRotation().Rotator(), true, true);
	}

	// 생존 도둑 수 감소
	if (UWorld* World = GetWorld())
	{
		if (ACYInGameState* CYGS = World->GetGameState<ACYInGameState>())
		{
			const int32 Alive = CYGS->GetAliveRobberCount();
			CYGS->UpdateAliveRobberCount(FMath::Max(0, Alive - 1));
		}
	}
}

bool UCYGameplayAbility_Interact_Arrest::FindJailTransform(FTransform& OutTransform) const
{
	UWorld* World = GetWorld();
	{
		if (!World) return false;
	}
	
	// 1) Tag=="JailTransForm" 을 가진 액터 우선 TODO : 특정 타입 or 다른 방식으로 위치 미리 가져오기(GameMode/State 등)
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor*	A = *It;
		if (A && A->Tags.Contains(FName("JailTransform")))
		{
			OutTransform = A->GetActorTransform();
			return true;
		}
	}

	return false;
}