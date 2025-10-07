#include "CYCharacterMovementComponent.h"

#include "GameFramework/Character.h"

UCYCharacterMovementComponent::UCYCharacterMovementComponent()
{
	      
}

float UCYCharacterMovementComponent::GetMaxSpeed() const
{
	// 사다리 타기 모드 체크
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(CMOVE_Climbing))
	{
		return MaxClimbSpeed;
	}

	// 기본 이동 모드는 부모 클래스 로직 사용
	return Super::GetMaxSpeed();
}

void UCYCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	const bool bNowClimbing = (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(CMOVE_Climbing));
	const bool bWasClimbing = (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == static_cast<uint8>(CMOVE_Climbing));

	if (bNowClimbing)
	{
		// 캐시 (사다리 "진입"시에만)
		DefaultGravityScale = GravityScale;
		DefaultBrakingFrictionFactor = BrakingFrictionFactor;
		bSavedOrientRotationToMovement = bOrientRotationToMovement;

		// 컨트롤러 회전 사용 여부는 Character 쪽 플래그를 저장/비활성화
		if (CharacterOwner)
		{
			// NOTE: 변수명은 기존 것을 재사용하지만 실제로는 bUseControllerRotationYaw를 캐시합니다.
			bSavedUseControllerDesiredRotation = CharacterOwner->bUseControllerRotationYaw;
			CharacterOwner->bUseControllerRotationYaw = false;
		}

		GravityScale = 0.f;
		BrakingFrictionFactor = 0.f;
		bOrientRotationToMovement = false;
		Velocity = FVector::ZeroVector;
		
		// 스냅 가드: 로컬(Autonomous) 또는 서버(Authority)에서만 + 데이터 유효할 때만
		const bool bCanSnap =
			(CharacterOwner && (CharacterOwner->IsLocallyControlled() || CharacterOwner->HasAuthority())) &&
			LadderActor.IsValid() &&
			RailLength > KINDA_SMALL_NUMBER;

		if (bCanSnap)
		{
			SnapToRailAndFace();
		}
	}
	else if (bWasClimbing)
	{
		// 캐시 복원 (사다리 "이탈"시에만)
		GravityScale = DefaultGravityScale;
		BrakingFrictionFactor = DefaultBrakingFrictionFactor;
		bOrientRotationToMovement = bSavedOrientRotationToMovement;

		if (CharacterOwner)
		{
			CharacterOwner->bUseControllerRotationYaw = bSavedUseControllerDesiredRotation;
		}

		// 안전장치
		bWantsToClimb = false;
		LadderActor.Reset();
		RailLength = 0.f;
		RailDirection = FVector::UpVector;
	}
}

void UCYCharacterMovementComponent::BeginClimbLadder(AActor* InLadder, const FVector& InStart, const FVector& InEnd, const FVector& InFacing, float InAttachSpot)
{
	if (!IsValid(InLadder) || !UpdatedComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginClimbLadder: Invalid Ladder or UpdatedComponent"));
		return;
	}

	LadderActor = InLadder;      
	LadderStart = InStart;    
	LadderEnd   = InEnd;  

	bWantsToClimb = true;
	
	// 레일 방향/길이 계산
	// RailDirection: 사다리 레일의 정규화된 방향 벡터 (입력 투영에 사용)
	RailDirection = (LadderEnd - LadderStart).GetSafeNormal();

	// RailLength: 사다리 레일의 총 길이 (cm)
	RailLength = (LadderEnd - LadderStart).Size();

	FVector RawFacing = InFacing;
	if (!RawFacing.Normalize())
	{
		RawFacing = FVector::ForwardVector; 
	}

	// 레일축에 직교한 사다리 평면 전방으로 정리
	// Facing을 RailDirection에 정사영하여 제거 → 완전 직교
	FVector FacingOnPlane = RawFacing - FVector::DotProduct(RawFacing, RailDirection) * RailDirection;
	if (!FacingOnPlane.Normalize())
	{
		// RawFacing이 RailDirection과 거의 평행한 극단 상황 → 임의의 직교축 생성
		const FVector AnyPerp = FVector::CrossProduct(
			RailDirection,
			(FMath::Abs(RailDirection.Z) < 0.99f ? FVector::UpVector : FVector::RightVector)
		);
		FacingOnPlane = AnyPerp.GetSafeNormal();
	}

	// 면 위 임의의 기준점(보통 사다리 시작점)에서 캐릭터 방향 벡터
	const FVector ToCharacter = (UpdatedComponent->GetComponentLocation() - LadderStart);

	// 법선이 캐릭터 반대로면 뒤집기
	if (FVector::DotProduct(FacingOnPlane, ToCharacter) < 0.f)
	{
		FacingOnPlane *= -1.f;
	}
	
	CharToLadderFacing = -FacingOnPlane;

	// 안전성 체크: LadderFacing이 0벡터면 기본값 사용
	if (CharToLadderFacing.IsNearlyZero())
	{
		CharToLadderFacing = FVector::ForwardVector;
	}

	// 초기 부착 위치 설정
	if (InAttachSpot >= 0.f)
	{
		// AttachSpot이 지정되면 해당 값 사용 (0 ~ RailLength 범위로 클램프)
		LadderAttachSpot = FMath::Clamp(InAttachSpot, 0.f, RailLength);
	}
	else
	{
		// AttachSpot이 음수면 현재 캐릭터 위치를 레일에 투영하여 계산
		// 사다리 중간에서 진입할 수 있도록 지원
		LadderAttachSpot = ProjectAttachSpot(UpdatedComponent->GetComponentLocation());
	}

	// OnMovementModeChanged()가 호출되어 중력 비활성화, 레일 스냅 등 처리
	SetMovementMode(MOVE_Custom, CMOVE_Climbing);
}

void UCYCharacterMovementComponent::EndClimbLadder(bool bStepOffTop)
{
	bWantsToClimb = false;
	
	// 현재는 간단히 이동 모드만 전환
	// TODO : 상단 탈출 시 추가 처리 가능 (예: 약간의 전방 임펄스)
	if (CharacterOwner && CharacterOwner->GetMovementBase())
	{
		// 바닥이 있으면 걷기 모드로 전환
		SetMovementMode(MOVE_Walking);
	}
	else
	{
		// 바닥이 없으면 낙하 모드로 전환
		SetMovementMode(MOVE_Falling);
	}
}

bool UCYCharacterMovementComponent::IsClimbingLadder() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(CMOVE_Climbing) && bWantsToClimb;
}

void UCYCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	// 사다리 타기 모드 체크
	if (CustomMovementMode == static_cast<uint8>(CMOVE_Climbing))
	{
		// 사다리 물리 처리
		PhysLadder(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UCYCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	// 유효성 검사
	if (DeltaTime < MIN_TICK_TIME || !UpdatedComponent || !LadderActor.IsValid() || !bWantsToClimb)
	{
		if (bWantsToClimb && !LadderActor.IsValid())
		{
			EndClimbLadder(false);
		}
		return;
	}
	
	// ConsumeInputVector(): AddMovementInput으로 누적된 입력 벡터 가져오기
	// 이 함수는 입력을 소비하므로 한 번만 호출해야 함
	FVector Pending = ConsumeInputVector();
	
	// 앞/뒤 의도를 CharToLadderFacing(수평) 방향으로 투영해서 스칼라로 얻는다.
	const float ForwardIntent = FVector::DotProduct(Pending.GetClampedToMaxSize(1.f), CharToLadderFacing);

	// (보정) 컨트롤러가 사다리와 거의 평행인 극단 상황에서 입력이 아주 작아질 수 있으니 한 번 더 클램프
	const float InputAxis = FMath::Clamp(ForwardIntent, -1.f, 1.f);

	// 레일 방향 속도 계산 (cm/s)
	const float SpeedAlongRail = InputAxis * MaxClimbSpeed;

	// LadderAttachSpot: 사다리 레일 상의 현재 위치 (0 ~ RailLength)
	// 적분: s(t+dt) = s(t) + v * dt
	LadderAttachSpot = FMath::Clamp(LadderAttachSpot + SpeedAlongRail * DeltaTime, 0.f, RailLength);

	// RailPos: 레일 상의 정확한 위치 (Start + Dir * s)
	const FVector RailPos = LadderStart + RailDirection * LadderAttachSpot;

	// 사다리 메시와 겹치지 않도록 CharToLadderFacing 방향으로 LadderStandOff만큼 위치를 이동
	const FVector DesiredPos = RailPos - CharToLadderFacing * LadderStandOff;

	// 캐릭터가 사다리를 바라봐야 하는 회전값을 계산
	const FQuat DesiredRot = FRotationMatrix::MakeFromXZ(CharToLadderFacing, RailDirection).ToQuat();

	// 현재 위치에서 목표 위치까지의 변위를 계산
	const FVector Delta = (DesiredPos - UpdatedComponent->GetComponentLocation());

	// MoveUpdatedComponent: 안전한 이동 (충돌 체크 포함)
	FHitResult Hit;
	MoveUpdatedComponent(Delta, DesiredRot, true, &Hit);

	Velocity = RailDirection * SpeedAlongRail;

	// ==========  상/하단 도달 체크  ==========
	// 현재는 Ability에서 처리중
	// 필요 시 델리게이트 브로드캐스트 추가 가능:
	// if (AtTop()) { OnReachedLadderTop.ExecuteIfBound(); }
	// if (AtBottom()) { OnReachedLadderBottom.ExecuteIfBound(); }
}

void UCYCharacterMovementComponent::SnapToRailAndFace()
{
	if (!UpdatedComponent)
	{
		return;
	}
	
	// 레일 상의 위치 계산
	const FVector RailPos = LadderStart + RailDirection * LadderAttachSpot;

	// 캐릭터 위치 (사다리에서 약간 떨어뜨림)
	const FVector DesiredPos = RailPos - CharToLadderFacing * LadderStandOff;

	// 캐릭터 회전 (사다리를 바라봄)
	const FQuat DesiredRot = FRotationMatrix::MakeFromXZ(CharToLadderFacing, RailDirection).ToQuat();

	// 즉시 텔레포트 (물리 충돌 무시)
	UpdatedComponent->SetWorldLocationAndRotation(DesiredPos, DesiredRot, false, nullptr, ETeleportType::TeleportPhysics);
}

float UCYCharacterMovementComponent::ProjectAttachSpot(const FVector& WorldPos) const
{
	if (RailLength <= KINDA_SMALL_NUMBER || RailDirection.IsNearlyZero())
	{
		return 0.f;
	}

	// WorldPos를 레일 시작점 기준으로 이동
	const FVector FromStart = WorldPos - LadderStart;

	// 레일 방향으로의 스칼라 투영 길이
	const float Spot = FVector::DotProduct(FromStart, RailDirection); 

	// 선분 [0, RailLength]로 클램프하여 실제 부착 지점 반환
	return FMath::Clamp(Spot, 0.f, RailLength);
}

void UCYCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// FLAG_Custom_0 비트 체크 (사다리 타기 의도)
	bWantsToClimb = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UCYCharacterMovementComponent::GetPredictionData_Client() const
{
	// 필요할 때만 할당
	if (ClientPredictionData == nullptr)
	{
		UCYCharacterMovementComponent* MutableThis = const_cast<UCYCharacterMovementComponent*>(this);
		// 예측 데이터 생성
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_CY(*this);

	}

	return ClientPredictionData;
}

FSavedMove_CY::FSavedMove_CY()
	: bWantsToClimb(false)
{
}

void FSavedMove_CY::Clear()
{
	Super::Clear();
	bWantsToClimb = false;
}

uint8 FSavedMove_CY::GetCompressedFlags() const
{
	// 부모 클래스의 플래그 가져오기
	uint8 Result = Super::GetCompressedFlags();

	// 사다리 타기 의도를 FLAG_Custom_0 비트로 설정
	if (bWantsToClimb)
	{
		Result |= FLAG_Custom_0;  // 비트 OR 연산으로 플래그 설정
	}

	return Result;
}

bool FSavedMove_CY::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	// NewMove를 FSavedMove_CY로 캐스팅
	const FSavedMove_CY* NewCY = static_cast<const FSavedMove_CY*>(NewMove.Get());

	// bWantsToClimb 상태가 동일하고, 부모 클래스 조건도 만족해야 병합 가능
	return (bWantsToClimb == NewCY->bWantsToClimb) && Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_CY::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	// 부모 클래스의 기본 데이터 저장 (위치, 속도, 회전 등)
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	// 커스텀 MovementComponent 캐스팅
	if (const auto* CYMovementComponent = Cast<UCYCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		// 현재 사다리 타는 중인지 저장
		bWantsToClimb = CYMovementComponent->bWantsToClimb;
	}
}

void FSavedMove_CY::PrepMoveFor(ACharacter* Character)
{
	// 부모 클래스의 기본 복원 (위치, 속도, 회전 등)
	Super::PrepMoveFor(Character);

	if (UCYCharacterMovementComponent* CY = Cast<UCYCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		// 필요 시 복원 로직 작성:
		// if (bWantsToClimb && !CY->IsClimbingLadder())
		// {
		//     // 예측 불일치 감지 및 처리
		// }
	}
}

FNetworkPredictionData_Client_CY::FNetworkPredictionData_Client_CY(const UCharacterMovementComponent& ClientMovement)
	: FNetworkPredictionData_Client_Character(ClientMovement)
{
	
}

FSavedMovePtr FNetworkPredictionData_Client_CY::AllocateNewMove()
{
	// 커스텀 SavedMove 할당 (스마트 포인터로 반환)
	return FSavedMovePtr(new FSavedMove_CY());
}


