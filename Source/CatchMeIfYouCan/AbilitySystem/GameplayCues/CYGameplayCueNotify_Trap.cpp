#include "AbilitySystem/GameplayCues/CYGameplayCueNotify_Trap.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

UCYGameplayCueNotify_Trap::UCYGameplayCueNotify_Trap()
{
}

bool UCYGameplayCueNotify_Trap::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	PlayTrapEffects(MyTarget, Parameters);
	
	UE_LOG(LogTemp, Log, TEXT("Trap effect played (Instant): %s"), *MyTarget->GetName());
	return true;
}

bool UCYGameplayCueNotify_Trap::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	PlayTrapEffects(MyTarget, Parameters);
	
	UE_LOG(LogTemp, Log, TEXT("Trap effect played (Duration Start): %s"), *MyTarget->GetName());
	return true;
}

void UCYGameplayCueNotify_Trap::PlayTrapEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return;

	// 소켓이 설정되어 있는지 확인
	bool bShouldAttach = (AttachSocketName != NAME_None);

	if (bShouldAttach)
	{
		// 소켓 부착 모드
		USceneComponent* AttachComponent = MyTarget->GetRootComponent();
		
		// 파티클 (Cascade)
		if (TriggerParticle)
		{
			UGameplayStatics::SpawnEmitterAttached(
				TriggerParticle,
				AttachComponent,
				AttachSocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}

		// 파티클 (Niagara)
		if (TriggerNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				TriggerNiagara,
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

		// 파티클 (Cascade)
		if (TriggerParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				MyTarget->GetWorld(),
				TriggerParticle,
				Location,
				FRotator::ZeroRotator,
				FVector(1.0f)
			);
		}

		// 파티클 (Niagara)
		if (TriggerNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				MyTarget->GetWorld(),
				TriggerNiagara,
				Location
			);
		}
	}

	// 사운드는 항상 위치 기반
	if (TriggerSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget->GetWorld(),
			TriggerSound,
			MyTarget->GetActorLocation()
		);
	}
}