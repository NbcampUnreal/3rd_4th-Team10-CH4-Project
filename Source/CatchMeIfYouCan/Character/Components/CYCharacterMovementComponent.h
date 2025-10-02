// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CYCharacterMovementComponent.generated.h"

class ACYLadderBase;

DECLARE_DELEGATE(FOnReachedLadderTop);
DECLARE_DELEGATE(FOnReachedLadderBottom);

UENUM(BlueprintType)
enum ECYCustomMovementMode
{
	CMOVE_None         UMETA(DisplayName = "None"),
	CMOVE_Climbing     UMETA(DisplayName = "Climbing"),
	CMOVE_MAX          UMETA(Hidden),
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHMEIFYOUCAN_API UCYCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCYCharacterMovementComponent();

	/** 사다리 등반 시작(Ability/RPC에서 호출) */
	UFUNCTION(BlueprintCallable, Category="CY|Movement|Ladder")
	void BeginClimbLadder(AActor* Ladder, const FVector& LadderStartWS, const FVector& LadderEndWS, const FVector& LadderFacingWS, float AttachS = -1.f);

	/** 사다리 등반 종료(Ability/RPC에서 호출) */
	UFUNCTION(BlueprintCallable, Category="CY|Movement|Ladder")
	void EndClimbLadder(bool bStepOffTop);

	/** 현재 등반 중인지 */
	UFUNCTION(BlueprintPure, Category="CY|Movement|Ladder")
	bool IsClimbingLadder() const
	{
		return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECYCustomMovementMode::CMOVE_Climbing);
	}

	// UMovementComponent / UCharacterMovementComponent
	virtual float GetMaxSpeed() const override;

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	//virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

private:
	// 실제 사다리 물리 처리
	void PhysLadder(float DeltaTime, int32 Iterations);

	// 사다리 선분에 스냅/정렬
	void SnapToRailAndFace();
	// 등반 레일에서의 파라미터 s(0 ~ 길이) 산출
	float ProjectAttachS(const FVector& WorldPos) const;
	// 좌우/전후 흔들림 보정(옵션 훅)
	void ConstrainToRail();

	// 상/하단 도달 체크 (필요 시 Ability에 이벤트 브로드캐스트 훅)
	bool AtTop() const;
	bool AtBottom() const;

private:
	// 사다리 참조/파라미터
	UPROPERTY(Transient) TWeakObjectPtr<AActor> LadderActor;
	FVector LadderStart = FVector::ZeroVector;
	FVector LadderEnd   = FVector::ZeroVector;
	FVector LadderFacing = FVector::ForwardVector;     // 월드 수평 방향(사다리 바라보는 방향)
	float   LadderAttachS = 0.f;                       // 레일 상 위치(0 ~ Length)

	// 튜닝 값
	UPROPERTY(EditAnywhere, Category="CY|Movement|Ladder")
	float MaxClimbSpeed = 180.f;

	UPROPERTY(EditAnywhere, Category="CY|Movement|Ladder")
	float LadderStandOff = 30.f;                       // 사다리 평면에서 살짝 떨어뜨리는 거리

	UPROPERTY(EditAnywhere, Category="CY|Movement|Ladder")
	float SnapStrength = 20.f;

	// 내부 캐시
	float RailLength = 0.f;
	FVector RailDir = FVector::UpVector;               // (End - Start).GetSafeNormal()
};




