#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "CYGameplayCueNotify_Consumable.generated.h"

class UParticleSystem;
class USoundBase;
class UNiagaraSystem;

UCLASS(BlueprintType, Blueprintable)
class CATCHMEIFYOUCAN_API UCYGameplayCueNotify_Consumable : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UCYGameplayCueNotify_Consumable();

	// 블루프린트에서 설정할 에셋들
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable Effects")
	UParticleSystem* UseParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable Effects")
	UNiagaraSystem* UseNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable Effects")
	USoundBase* UseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable Effects")
	FName AttachSocketName = NAME_None;

protected:
	// Instant Effect용 (Heal)
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	
	// Duration Effect용 (SpeedBoost, Invisibility)
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

private:
	void PlayConsumableEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const;
};
