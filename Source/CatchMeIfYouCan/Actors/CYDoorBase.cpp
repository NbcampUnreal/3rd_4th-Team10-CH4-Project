// Fill out your copyright notice in the Description page of Project Settings.


#include "CYDoorBase.h"

#include "AbilitySystem/CYCombatGameplayTags.h"
#include "Character/CYCharacterBase.h"
#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"

ACYDoorBase::ACYDoorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	// 왼쪽 문짝 메시 컴포넌트
	LeftMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftMeshComponent"));
	LeftMeshComponent->SetupAttachment(GetRootComponent());
	// 상호작용 전용 콜리전 프로필 지정
	LeftMeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	LeftMeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftMeshComponent->SetCanEverAffectNavigation(true);

	// 왼쪽 문짝 메시 컴포넌트
	RightMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMeshComponent"));
	RightMeshComponent->SetupAttachment(GetRootComponent());
	// 상호작용 전용 콜리전 프로필 지정
	RightMeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	RightMeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	RightMeshComponent->SetCanEverAffectNavigation(true);
}

void ACYDoorBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DoorState);
}

bool ACYDoorBase::CanInteraction(const FCYInteractionQuery& InteractionQuery) const
{
	if (!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}

	// 요청 플레이어가 감옥 상태면 문 상호작용 불가
	if (const ACYCharacterBase* Requester = Cast<ACYCharacterBase>(InteractionQuery.RequestingAvatar.Get()))
	{
		if (Requester->HasGameplayTag(CYGameplayTags::State_Jail))
		{
			return false;
		}
	}

	return true;
}

FCYInteractionInfo ACYDoorBase::GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const
{
	// 문 상태에 따라 상호작용 정보를 반환
	switch (DoorState)
	{
	case ECYDoorState::Open_Forward:
	case ECYDoorState::Open_Backward:
		return OpenedInteractionInfo;
		
	case ECYDoorState::Close:
		return ClosedInteractionInfo;
		
	default:
		return FCYInteractionInfo();
	}
}

void ACYDoorBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	// 실제 메시가 설정된 경우에만 컴포넌트를 반환
	if (LeftMeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(LeftMeshComponent);
	}

	if (RightMeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(RightMeshComponent);
	}
}

void ACYDoorBase::SetDoorState(ECYDoorState NewDoorState)
{
	if (!HasAuthority() || NewDoorState == DoorState)
	{
		return;
	}
	
	DoorState = NewDoorState;
	OnRep_DoorState();
}

void ACYDoorBase::OnRep_DoorState()
{
	OnDoorStateChanged(DoorState);
}

