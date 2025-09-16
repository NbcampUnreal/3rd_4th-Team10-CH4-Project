// Fill out your copyright notice in the Description page of Project Settings.


#include "CYPlayerState.h"

#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CYVitalSet.h"

ACYPlayerState::ACYPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCYAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	VitalSet = CreateDefaultSubobject<UCYVitalSet>(TEXT("VitalSet"));
	SetNetUpdateFrequency(100.0f);
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
