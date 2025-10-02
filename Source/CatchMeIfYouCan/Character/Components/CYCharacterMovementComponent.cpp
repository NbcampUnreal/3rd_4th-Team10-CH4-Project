// Fill out your copyright notice in the Description page of Project Settings.


#include "CYCharacterMovementComponent.h"

#include "GameFramework/Character.h"

UCYCharacterMovementComponent::UCYCharacterMovementComponent()
{
	// 기본 튜닝
	MaxClimbSpeed = 180.f;
	LadderStandOff = 30.f;
	SnapStrength = 20.f;
}

float UCYCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECYCustomMovementMode::CMOVE_Climbing))
	{
		return MaxClimbSpeed;
	}
	return Super::GetMaxSpeed();
}

void UCYCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	const bool bNowLadder = (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECYCustomMovementMode::CMOVE_Climbing));

	if (bNowLadder)
	{
		// 중력 해제 및 초기화
		GravityScale = 0.f;
		BrakingFrictionFactor = 0.f;
		Velocity = FVector::ZeroVector;

		// 등반 진입 즉시 위치/회전 스냅
		SnapToRailAndFace();
	}
	else
	{
		// 정상 상태로 복구
		GravityScale = 1.f;
		BrakingFrictionFactor = 1.f;

		// 사다리 캐시 정리
		LadderActor.Reset();
		RailLength = 0.f;
		RailDir = FVector::UpVector;
	}
}

void UCYCharacterMovementComponent::BeginClimbLadder(AActor* Ladder, const FVector& InStart, const FVector& InEnd, const FVector& InFacing, float AttachSIn)
{
	if (!IsValid(Ladder) || !UpdatedComponent)
	{
		return;
	}

	LadderActor = Ladder;
	LadderStart = InStart;
	LadderEnd   = InEnd;
	RailDir = (LadderEnd - LadderStart).GetSafeNormal();
	RailLength = (LadderEnd - LadderStart).Size();

	// 수평 바라보는 방향(피치 제거)
	LadderFacing = FVector(InFacing.X, InFacing.Y, 0.f).GetSafeNormal();
	if (LadderFacing.IsNearlyZero())
	{
		LadderFacing = FVector::ForwardVector;
	}

	// 초기 s 위치
	LadderAttachS = (AttachSIn >= 0.f) ? FMath::Clamp(AttachSIn, 0.f, RailLength)
	                                   : ProjectAttachS(UpdatedComponent->GetComponentLocation());

	// 모드 전환
	SetMovementMode(MOVE_Custom, static_cast<uint8>(ECYCustomMovementMode::CMOVE_Climbing));
}

void UCYCharacterMovementComponent::EndClimbLadder(bool bStepOffTop)
{
	// 필요 시 상단 스텝오프 처리(약간의 임펄스 등)
	// 여기서는 간단히 워킹/폴링으로 복귀
	if (CharacterOwner && CharacterOwner->GetMovementBase())
	{
		SetMovementMode(MOVE_Walking);
	}
	else
	{
		SetMovementMode(MOVE_Falling);
	}
}

void UCYCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (CustomMovementMode == static_cast<uint8>(ECYCustomMovementMode::CMOVE_Climbing))
	{
		PhysLadder(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UCYCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME || !UpdatedComponent || !LadderActor.IsValid())
	{
		return;
	}

	// 1) 입력 취합: PendingInputVector를 등반 축(RailDir)에 사영하여 상/하 입력을 추출
	FVector Pending = ConsumeInputVector(); // AddMovementInput로 들어온 입력 누적치
	float AxisAlongRail = FVector::DotProduct(Pending.GetClampedToMaxSize(1.f), RailDir); // -1 ~ 1 근사
	const float SpeedAlongRail = AxisAlongRail * MaxClimbSpeed;

	// 2) s 파라미터 적분 (경계 클램프)
	LadderAttachS = FMath::Clamp(LadderAttachS + SpeedAlongRail * DeltaTime, 0.f, RailLength);

	// 3) 목표 위치/회전 산출(사다리 평면에서 약간 띄우기)
	const FVector RailPos = LadderStart + RailDir * LadderAttachS;
	const FVector DesiredPos = RailPos - LadderFacing * LadderStandOff; // 사다리에서 살짝 떨어짐
	const FQuat DesiredRot = FRotationMatrix::MakeFromXZ(LadderFacing, FVector::UpVector).ToQuat();

	// 4) 스냅 + 이동(안정성 위해 MoveUpdatedComponent 사용)
	const FVector Delta = (DesiredPos - UpdatedComponent->GetComponentLocation());
	FHitResult Hit;
	MoveUpdatedComponent(Delta, DesiredRot, true, &Hit);

	// 5) 수직 속도(정보 용도) & 제동
	Velocity = RailDir * SpeedAlongRail;

	// 6) 상/하단 도달 시 후처리 훅 (여기서는 모드만 유지, Ability가 이벤트 받을 수 있게 델리게이트 훅 추가 권장)
	// if (AtTop()) { ... }
	// if (AtBottom()) { ... }
}

void UCYCharacterMovementComponent::SnapToRailAndFace()
{
	if (!UpdatedComponent) return;

	const FVector RailPos = LadderStart + RailDir * LadderAttachS;
	const FVector DesiredPos = RailPos - LadderFacing * LadderStandOff;
	const FQuat DesiredRot = FRotationMatrix::MakeFromXZ(LadderFacing, FVector::UpVector).ToQuat();

	UpdatedComponent->SetWorldLocationAndRotation(DesiredPos, DesiredRot, false, nullptr, ETeleportType::TeleportPhysics);
}

float UCYCharacterMovementComponent::ProjectAttachS(const FVector& WorldPos) const
{
	const FVector Rail = (LadderEnd - LadderStart);
	const float Len = Rail.Size();
	if (Len <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	const FVector Dir = Rail / Len;
	const float S = FVector::DotProduct(WorldPos - LadderStart, Dir);
	return FMath::Clamp(S, 0.f, Len);
}

void UCYCharacterMovementComponent::ConstrainToRail()
{
	// 필요시 lateral 흔들림 보정 로직 추가 가능 (현재는 Snap에서 해결)
}

bool UCYCharacterMovementComponent::AtTop() const
{
	return LadderAttachS >= (RailLength - KINDA_SMALL_NUMBER);
}

bool UCYCharacterMovementComponent::AtBottom() const
{
	return LadderAttachS <= KINDA_SMALL_NUMBER;
}

void UCYCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// 클라 예측 힌트용 플래그 복원 (필요 시 사용)
	const bool bClimb = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	// 일반적으로 여기서 강제 모드 전환을 하지는 않는다.
	// BeginClimbLadder/EndClimbLadder는 Ability/RPC에서 관리.
}

