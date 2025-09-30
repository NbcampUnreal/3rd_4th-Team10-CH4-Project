#include "AbilitySystem/GameplayCues/CYGameplayCueNotify_Consumable.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

UCYGameplayCueNotify_Consumable::UCYGameplayCueNotify_Consumable()
{
}

bool UCYGameplayCueNotify_Consumable::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	PlayConsumableEffects(MyTarget, Parameters);
	
	UE_LOG(LogTemp, Log, TEXT("Consumable effect played (Instant): %s"), *MyTarget->GetName());
	return true;
}

bool UCYGameplayCueNotify_Consumable::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	PlayConsumableEffects(MyTarget, Parameters);
	
	UE_LOG(LogTemp, Log, TEXT("Consumable effect played (Duration Start): %s"), *MyTarget->GetName());
	return true;
}

void UCYGameplayCueNotify_Consumable::PlayConsumableEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return;

	// 소켓이 설정되어 있는지 확인
	bool bShouldAttach = (AttachSocketName != NAME_None);
	
	if (bShouldAttach)
	{
		// 소켓 부착 모드
		USceneComponent* AttachComponent = MyTarget->GetRootComponent();
		
		// 파티클 (Cascade) - 부착
		if (UseParticle)
		{
			UGameplayStatics::SpawnEmitterAttached(
				UseParticle,
				AttachComponent,
				AttachSocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}

		// 파티클 (Niagara) - 부착
		if (UseNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				UseNiagara,
				AttachComponent,
				AttachSocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}
	}
	else
	{
		// 위치 기반 모드
		FVector Location;
		if (Parameters.Location.IsZero())
		{
			Location = MyTarget->GetActorLocation();
		}
		else
		{
			Location = FVector(Parameters.Location);
		}

		// 파티클 (Cascade) - 위치
		if (UseParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				MyTarget->GetWorld(),
				UseParticle,
				Location,
				FRotator::ZeroRotator,
				FVector(1.0f)
			);
		}

		// 파티클 (Niagara) - 위치
		if (UseNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				MyTarget->GetWorld(),
				UseNiagara,
				Location
			);
		}
	}

	// 사운드는 항상 위치 기반
	if (UseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget->GetWorld(),
			UseSound,
			MyTarget->GetActorLocation()
		);
	}
}