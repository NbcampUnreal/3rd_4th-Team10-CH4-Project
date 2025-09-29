#include "CYRobberCharacter.h"

#include "AbilitySystem/CYCombatGameplayTags.h"
#include "Player/CYPlayerState.h"

ACYRobberCharacter::ACYRobberCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void ACYRobberCharacter::BeginPlay()
{
	Super::BeginPlay();

	// TODO : 함수화로 부착된 mesh들 CustomDepthStencilValue값 설정
	GetMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);
}

FCYInteractionInfo ACYRobberCharacter::GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const
{
	// TODO : 현재는 경찰이 도둑을 잡는 상호작용만 존재
	return ArrestedInteractionInfo;
}

void ACYRobberCharacter::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (USkeletalMeshComponent* MainMesh = GetMesh())
	{
		OutMeshComponents.Add(MainMesh);
	}
	// TODO : 필요시 추가 장착 메쉬 등록
}

bool ACYRobberCharacter::CanInteraction(const FCYInteractionQuery& InteractionQuery) const
{
	AController* RequestingController = InteractionQuery.RequestingController.Get();
	AActor* RequestingActor = InteractionQuery.RequestingAvatar.Get();
	const ACYPlayerState* RequestingPS = RequestingController ? RequestingController->GetPlayerState<ACYPlayerState>() : nullptr;
	if (!RequestingActor || !RequestingPS)
	{
		return false;
	}

	switch (RequestingPS->GetTeamRole())
	{
		case ECYTeamRole::Cop:
		{
			if (CanArrested(InteractionQuery))
			{
				return ArrestedInteractionInfo.AbilityToGrant != nullptr;
			}
			break;
		}
		default:
		{
			break;
		}
	}
	return false;
}

bool ACYRobberCharacter::CanArrested(const FCYInteractionQuery& InteractionQuery) const
{
	ACYPlayerState* CYPS = GetPlayerState<ACYPlayerState>();

	if (!CYPS)
	{
		return false;
	}
	
	// 1) 내가 도둑이어야 함 TODO : GameplayTag로 팀 체크 고려
	if (CYPS->GetTeamRole() != ECYTeamRole::Robber)
	{
		return false;
	}
	// 2) 내가 스턴 상태(태그로 관리)
	if (!HasGameplayTag(CYGameplayTags::State_Stunned))
	{
		return false;
	}
	// 3) 이미 잡힘/감옥 상태면 불가
	if (HasGameplayTag(CYGameplayTags::State_Jail))
	{
		return false;
	}

	return true;
}

