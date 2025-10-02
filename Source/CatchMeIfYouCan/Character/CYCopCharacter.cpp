// Fill out your copyright notice in the Description page of Project Settings.


#include "CYCopCharacter.h"

#include "Blueprint/UserWidget.h"
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
	if (!PC || !PC->IsLocalController()) return;

	if (bShow)
	{
		if (!WarningWidget && WarningWidgetClass)
		{
			WarningWidget = CreateWidget<UUserWidget>(PC, WarningWidgetClass);
			if (WarningWidget)
			{
				WarningWidget->AddToViewport(100);
			}
		}
	}
	else
	{
		if (WarningWidget)
		{
			FTimerHandle RemoveTimer;
			GetWorld()->GetTimerManager().SetTimer(RemoveTimer, [this]()
			{
				if (WarningWidget)
				{
					WarningWidget->RemoveFromParent();
					WarningWidget = nullptr;
				}
			}, 0.3f, false);
		}
	}
}