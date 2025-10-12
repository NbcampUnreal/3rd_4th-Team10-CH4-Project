#include "CYGameplayAbility_Jump_Ladder.h"

#include "CYLogChannels.h"
#include "Character/CYCharacterBase.h"
#include "Character/Components/CYCharacterMovementComponent.h"

UCYGameplayAbility_Jump_Ladder::UCYGameplayAbility_Jump_Ladder()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ECYAbilityActivationPolicy::Manual;
}

void UCYGameplayAbility_Jump_Ladder::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACYCharacterBase* CYCharacter = GetCYCharacterFromActorInfo();
	if (!CYCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCYCharacterMovementComponent* MovementComp = Cast<UCYCharacterMovementComponent>(CYCharacter->GetCharacterMovement());
	if (!MovementComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 이 시점에는 사다리 정보가 아직 유효함
	FVector BackwardDirection = -MovementComp->GetCharToLadderFacing();

	if (BackwardDirection.IsNearlyZero())
	{
		BackwardDirection = -CYCharacter->GetActorForwardVector();
	}

	FRotator TargetRotation = BackwardDirection.Rotation();
	TargetRotation.Pitch = 0.f; 
	TargetRotation.Roll = 0.f;
	
	CYCharacter->SetActorRotation(TargetRotation);

	if (APlayerController* PC = Cast<APlayerController>(CYCharacter->GetController()))
	{
		if (PC->IsLocalController() && bRotateCamera)
		{
			FRotator CurrentControlRotation = PC->GetControlRotation();
			
			// Yaw만 변경 (Pitch는 플레이어가 보던 각도 유지)
			FRotator NewControlRotation = CurrentControlRotation;
			NewControlRotation.Yaw = TargetRotation.Yaw;
			
			PC->SetControlRotation(NewControlRotation);
		}
	}
	
	// 점프 속도 벡터 구성
	FVector LaunchVelocity = BackwardDirection * LadderBackwardJumpForce + FVector::UpVector * LadderUpwardJumpForce;
	
	// LaunchCharacter로 즉시 속도 적용
	CYCharacter->LaunchCharacter(LaunchVelocity, true, true);
	
	UE_LOG(LogCY, Warning, TEXT("[%s] Ladder jump executed: Direction=%s, Velocity=%s"),
		HasAuthority(&ActivationInfo) ? TEXT("Server") : TEXT("Client"),
		*BackwardDirection.ToString(), *LaunchVelocity.ToString());

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}