// Fill out your copyright notice in the Description page of Project Settings.


#include "CYCopCharacter.h"

ACYCopCharacter::ACYCopCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FCYInteractionInfo ACYCopCharacter::GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const
{
	// TODO : 추후 경찰 캐릭터와도 상호작용 기능 추가시 해당 부분 설계
	return FCYInteractionInfo();
}

void ACYCopCharacter::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	// TODO : 추후 경찰 캐릭터와도 상호작용 기능 추가시 해당 부분 설계
}

bool ACYCopCharacter::CanInteraction(const FCYInteractionQuery& InteractionQuery) const
{
	// TODO : 추후 경찰 캐릭터와도 상호작용 기능 추가시 해당 부분 설계
	return false;
}

void ACYCopCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
