// Fill out your copyright notice in the Description page of Project Settings.


#include "CYSafe.h"

#include "Character/CYCharacterBase.h"
#include "GameModes/InGame/CYInGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CYPlayerState.h"

ACYSafe::ACYSafe(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SafeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SafeMesh"));
	SetRootComponent(SafeMesh);
	SafeMesh->SetCollisionProfileName(TEXT("Interactable"));
    
	// 일회성 상호작용 설정
	bShouldConsume = true;
}

bool ACYSafe::CanInteraction(const FCYInteractionQuery& InteractionQuery) const
{
	// 부모 클래스 체크 (bWasConsumed 검증)
	if (!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}
	
	// 도둑만 상호작용 가능
	if (const ACYCharacterBase* Requester = Cast<ACYCharacterBase>(InteractionQuery.RequestingAvatar.Get()))
	{
		if (const ACYPlayerState* PS = Requester->GetPlayerState<ACYPlayerState>())
		{
			return PS->GetTeamRole() == ECYTeamRole::Robber;
		}
	}
	return false;
}

FCYInteractionInfo ACYSafe::GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const
{
	return SafeInteractionInfo;
}

void ACYSafe::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (SafeMesh->GetStaticMesh())
	{
		OutMeshComponents.Add(SafeMesh);
	}
}

void ACYSafe::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor); // bWasConsumed = true 처리
    
	// GameState에 알림 (서버에서만)
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (ACYInGameState* GameState = World->GetGameState<ACYInGameState>())
			{
				// 열린 금고 +1
				GameState->UpdateOpenedSafeCount(1); 
			}
		}

		OnRep_WasConsumed();
		Multicast_PlayOpenEffects();
	}
}

void ACYSafe::OnRep_WasConsumed()
{
	if (bWasConsumed)
	{
		UpdateSafeMaterial();
	}
}

void ACYSafe::UpdateSafeMaterial()
{
	if (!SafeMesh)
	{
		return;
	}
	
	if (bWasConsumed && OpenedMaterial)
	{
		SafeMesh->SetMaterial(0, OpenedMaterial);
	}
}

void ACYSafe::PlayOpenEffects()
{
	if (OpenParticle)
	{
		FVector SpawnLocation = GetActorLocation();
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			OpenParticle,
			SpawnLocation + ExplosionZOffset,
			GetActorRotation(),
			ExplosionScale,
			true);
	}

	if (OpenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			OpenSound,
			GetActorLocation());
	}
}

void ACYSafe::Multicast_PlayOpenEffects_Implementation()
{
	PlayOpenEffects();
}

