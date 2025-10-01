// Fill out your copyright notice in the Description page of Project Settings.


#include "CYCopCharacter.h"

#include "UI/WidgetController/CYOverlayWidgetController.h"

class ACYPlayerState;

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

void ACYCopCharacter::Client_ShowRobberDetectedWarning_Implementation(bool bShow, AActor* DetectedThief)
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC && PC->IsLocalController())
	{
		// TODO: 새로 만들 Cop 전용 HUD 클래스를 가져와서 로직을 구현해야 합니다.
	}
}