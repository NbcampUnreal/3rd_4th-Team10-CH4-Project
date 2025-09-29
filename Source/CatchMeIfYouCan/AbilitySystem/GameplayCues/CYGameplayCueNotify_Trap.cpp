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

	// 사운드
	if (TriggerSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget->GetWorld(),
			TriggerSound,
			Location
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("Trap effect played at %s"), *Location.ToString());
	return true;
}

bool UCYGameplayCueNotify_Trap::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

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

	// 사운드
	if (TriggerSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
		   MyTarget->GetWorld(),
		   TriggerSound,
		   Location
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("Duration Trap effect played at %s (OnActive)"), *Location.ToString());
	return true;
}