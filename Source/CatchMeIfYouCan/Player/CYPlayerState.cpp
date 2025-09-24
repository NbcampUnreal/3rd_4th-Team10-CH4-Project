// Fill out your copyright notice in the Description page of Project Settings.


#include "CYPlayerState.h"

#include "CYLogChannels.h"
#include "CYPlayerController.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CYVitalSet.h"
#include "Character/CYCharacterBase.h"
#include "Character/CYPawnData.h"
#include "Net/UnrealNetwork.h"

ACYPlayerState::ACYPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCYAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	VitalSet = CreateDefaultSubobject<UCYVitalSet>(TEXT("VitalSet"));
	SetNetUpdateFrequency(100.0f);
}

void ACYPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(ThisClass, TeamRole);
	DOREPLIFETIME(ThisClass, PawnData);
}

UAbilitySystemComponent* ACYPlayerState::GetAbilitySystemComponent() const
{
	return GetCYAbilitySystemComponent();
}

void ACYPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	// 초기 Ability Actor Info를 설정(현재 시점에서 GetPawn은 nullptr)
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
	
}

void ACYPlayerState::SetTeamRole(ECYTeamRole NewTeamRole)
{
	if (!HasAuthority())
	{
		return;
	}
	
	TeamRole = NewTeamRole;
}

void ACYPlayerState::SetPawnData(UCYPawnData* NewPawnData)
{
	if (!HasAuthority())
	{
		return;
	}

	PawnData = NewPawnData;
	TeamRole = PawnData->TeamRole;

	// 서버: Pawn이 있으면 즉시 초기화 시도
	if (APawn* MyPawn = GetPawn())
	{
		if (ACYCharacterBase* Character = Cast<ACYCharacterBase>(MyPawn))
		{
			Character->TryInitializeAbilitySetsWithPawnData();
		}
	}

	// 리슨 서버인 경우 처리
	if (GetNetMode() == NM_ListenServer)
	{
		NotifyControllerPawnDataReady();
	}
}

void ACYPlayerState::OnRep_TeamRole()
{
	// TODO : 함수 삭제 예정
	UE_LOG(LogCY, Warning, TEXT("Player %s assigned to team: %s"), 
		*GetPlayerName(),
		TeamRole == ECYTeamRole::Cop ? TEXT("Cop") : TEXT("Robber"));
}

void ACYPlayerState::OnRep_PawnData()
{
	if (PawnData)
	{
		NotifyControllerPawnDataReady();
	}
}

void ACYPlayerState::NotifyControllerPawnDataReady()
{
	if (ACYPlayerController* PC = Cast<ACYPlayerController>(GetOwner()))
	{
		if (PC->IsLocalController())
		{
			PC->OnPawnDataReady();
		}
	}
}
