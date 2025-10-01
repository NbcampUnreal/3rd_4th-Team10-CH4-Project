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

	// TODO : StencilValue 처리를 상호작용 타입에 따라 적용되도록 처리하는 위치나 구조 변경 고민
	GetMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);
	GetHelmetMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);
	GetEyewearMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);
	GetChestMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);
	GetLegsMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);
	GetFootwearMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);	
}

FCYInteractionInfo ACYRobberCharacter::GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const
{
	// TODO : 현재는 경찰이 도둑을 잡는 상호작용만 존재
	return ArrestedInteractionInfo;
}

void ACYRobberCharacter::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (GetMesh()->GetSkeletalMeshAsset())
	{
		OutMeshComponents.Add(GetMesh());
	}

	if (GetHelmetMesh()->GetSkeletalMeshAsset())
	{
		OutMeshComponents.Add(GetHelmetMesh());
	}

	if (GetEyewearMesh()->GetSkeletalMeshAsset())
	{
		OutMeshComponents.Add(GetEyewearMesh());
	}

	if (GetChestMesh()->GetSkeletalMeshAsset())
	{
		OutMeshComponents.Add(GetChestMesh());
	}

	if (GetLegsMesh()->GetSkeletalMeshAsset())
	{
		OutMeshComponents.Add(GetLegsMesh());
	}

	if (GetFootwearMesh()->GetSkeletalMeshAsset())
	{
		OutMeshComponents.Add(GetFootwearMesh());
	}
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

