// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "CYAnimInstance.generated.h"

class ACYCharacterBase;

struct FCYAnimInstanceProxy : FAnimInstanceProxy
{
	FCYAnimInstanceProxy(UAnimInstance* Instance);

	// 게임 스레드에서 데이터를 안전하게 수집하는 함수
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	
	// 워커 스레드에서 안전하게 사용할 캐시된 데이터들
	FVector CachedVelocity;
	FVector CachedAcceleration;
	FRotator CachedActorRotation;
	bool bCachedIsFalling;
	bool bCachedIsMovingOnGround;
	bool bCachedIsClimbing;

private:
	void UpdateMovementData(const ACYCharacterBase* Character);
	void UpdateRotationData(const ACYCharacterBase* Character);
};

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;

private:
	void UpdateRotationValues(const FCYAnimInstanceProxy& Proxy);
	void UpdateMovementStates(const FCYAnimInstanceProxy& Proxy);
	void UpdateVelocityValues(const FCYAnimInstanceProxy& Proxy);
	void UpdateAccelerationValues(const FCYAnimInstanceProxy& Proxy);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasVelocity;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasAcceleration;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsJumping;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsOnGround;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float FallSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsClimbing;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float ClimbSpeed;
};
