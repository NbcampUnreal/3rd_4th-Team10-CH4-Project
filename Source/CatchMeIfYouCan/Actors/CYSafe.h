// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/CYWorldInteractable.h"
#include "CYSafe.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYSafe : public ACYWorldInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACYSafe(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 이미 열렸는지 상태
	UFUNCTION(BlueprintPure)
	bool IsOpened() const { return bWasConsumed; }
    
protected:
	virtual bool CanInteraction(const FCYInteractionQuery& InteractionQuery) const override;
	virtual FCYInteractionInfo GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;
    
	// 금고 열기 성공 시 호출 (서버에서만)
	virtual void OnInteractionSuccess(AActor* Interactor) override;

	virtual void OnRep_WasConsumed() override;
	void UpdateSafeMaterial();

	void PlayOpenEffects();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOpenEffects();
    
protected:
	UPROPERTY(EditDefaultsOnly, Category="Info")
	FCYInteractionInfo SafeInteractionInfo;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SafeMesh;

	UPROPERTY(EditDefaultsOnly, Category="Safe|Material")
	TObjectPtr<UMaterialInterface> OpenedMaterial;

	UPROPERTY(EditDefaultsOnly, Category="Safe|VFX")
	TObjectPtr<UParticleSystem> OpenParticle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe|VFX")
	FVector ExplosionScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe|VFX")
	FVector ExplosionZOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category="Safe|SFX")
	TObjectPtr<USoundBase> OpenSound;
};
