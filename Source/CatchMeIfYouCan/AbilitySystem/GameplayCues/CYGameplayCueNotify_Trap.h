#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "CYGameplayCueNotify_Trap.generated.h"

class UParticleSystem;
class USoundBase;
class UNiagaraSystem;

UCLASS(BlueprintType, Blueprintable)
class CATCHMEIFYOUCAN_API UCYGameplayCueNotify_Trap : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UCYGameplayCueNotify_Trap();

	// 블루프린트에서 설정할 에셋들
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trap Effects")
	UParticleSystem* TriggerParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trap Effects")
	UNiagaraSystem* TriggerNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trap Effects")
	USoundBase* TriggerSound;

	// 부착할 소켓 이름 (None이면 위치 기반)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trap Effects")
	FName AttachSocketName = NAME_None;

protected:
	// Instant Effect용 (Damage 트랩)
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	
	// Duration Effect용 (Slow/Freeze 트랩)
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

private:
	void PlayTrapEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const;
};