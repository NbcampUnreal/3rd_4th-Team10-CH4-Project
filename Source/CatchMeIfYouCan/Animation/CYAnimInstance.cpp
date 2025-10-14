// Fill out your copyright notice in the Description page of Project Settings.


#include "CYAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Character/CYCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

FCYAnimInstanceProxy::FCYAnimInstanceProxy(UAnimInstance* Instance)
	: FAnimInstanceProxy(Instance)
{
}

void FCYAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);

	if (!InAnimInstance)
	{
		return;
	}
	ACYCharacterBase* OwningCharacter = Cast<ACYCharacterBase>(InAnimInstance->GetOwningActor());
	if (!IsValid(OwningCharacter))
	{
		return;
	}
	
	UpdateMovementData(OwningCharacter);
	UpdateRotationData(OwningCharacter);
}

void FCYAnimInstanceProxy::UpdateMovementData(const ACYCharacterBase* Character)
{
	if (!Character) return;
    
	CachedVelocity = Character->GetVelocity();
	bCachedIsClimbing = Character->IsClimbing();
    
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		bCachedIsFalling = MovementComponent->IsFalling();
		bCachedIsMovingOnGround = MovementComponent->IsMovingOnGround();
		CachedAcceleration = MovementComponent->GetCurrentAcceleration();
	}
	else
	{
		bCachedIsFalling = false;
		bCachedIsMovingOnGround = false;
		CachedAcceleration = FVector::ZeroVector;
	}
}

void FCYAnimInstanceProxy::UpdateRotationData(const ACYCharacterBase* Character)
{
	if (!Character) return;
    
	CachedActorRotation = Character->GetActorRotation();
}

FAnimInstanceProxy* UCYAnimInstance::CreateAnimInstanceProxy()
{
	return new FCYAnimInstanceProxy(this);
}

void UCYAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	delete InProxy;
}

void UCYAnimInstance::UpdateRotationValues(const FCYAnimInstanceProxy& Proxy)
{
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(
		Proxy.CachedVelocity, 
		Proxy.CachedActorRotation
	);
}

void UCYAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	const auto& BaseAnimProxy = GetProxyOnAnyThread<FCYAnimInstanceProxy>();

	UpdateMovementStates(BaseAnimProxy);
	UpdateVelocityValues(BaseAnimProxy);
	UpdateAccelerationValues(BaseAnimProxy);
	
}

void UCYAnimInstance::UpdateMovementStates(const FCYAnimInstanceProxy& Proxy)
{
	bIsInAir = Proxy.bCachedIsFalling;
	bIsOnGround = Proxy.bCachedIsMovingOnGround;
	bIsClimbing = Proxy.bCachedIsClimbing;

	bIsJumping = bIsInAir && Proxy.CachedVelocity.Z > 0;
	bIsFalling = bIsInAir && Proxy.CachedVelocity.Z <= 0;

	GroundSpeed = Proxy.CachedVelocity.Size2D();
	ClimbSpeed = Proxy.CachedVelocity.Size();
	FallSpeed = Proxy.CachedVelocity.Z;
}

void UCYAnimInstance::UpdateVelocityValues(const FCYAnimInstanceProxy& Proxy)
{
	const FVector WorldVelocity2D = Proxy.CachedVelocity * FVector(1.f, 1.f, 0.f);
	const FVector LocalVelocity2D = Proxy.CachedActorRotation.UnrotateVector(WorldVelocity2D);
    
	bHasVelocity = !UKismetMathLibrary::NearlyEqual_FloatFloat(
		UKismetMathLibrary::VSizeXYSquared(LocalVelocity2D), 
		0.0f
	);
}

void UCYAnimInstance::UpdateAccelerationValues(const FCYAnimInstanceProxy& Proxy)
{
	const FVector WorldAcceleration2D = Proxy.CachedAcceleration * FVector(1.f, 1.f, 0.f);
	const FVector LocalAcceleration2D = Proxy.CachedActorRotation.UnrotateVector(WorldAcceleration2D);
    
	bHasAcceleration = !UKismetMathLibrary::NearlyEqual_FloatFloat(
		UKismetMathLibrary::VSizeXYSquared(LocalAcceleration2D), 
		0.0f
	);
}