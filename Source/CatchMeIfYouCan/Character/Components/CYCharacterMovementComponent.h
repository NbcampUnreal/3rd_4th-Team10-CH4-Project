#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CYCharacterMovementComponent.generated.h"

class ACYLadderBase;

DECLARE_DELEGATE(FOnReachedLadderTop);
DECLARE_DELEGATE(FOnReachedLadderBottom);

/**
 * 커스텀 이동 모드 열거형
 * UE의 기본 이동 모드(Walking, Falling, Flying, Swimming) 외에
 * MOVE_Custom 모드 사용 시 CustomMovementMode 값으로 구분
 */
UENUM(BlueprintType)
enum ECYCustomMovementMode
{
	CMOVE_None         UMETA(DisplayName = "None"),      // 커스텀 모드 없음
	CMOVE_Climbing     UMETA(DisplayName = "Climbing"),  // 사다리 타기 모드
	CMOVE_MAX          UMETA(Hidden),                    // 열거형 최대값 (내부용)
};

/**
 * 주요 기능:
 * 1. 사다리 타기 커스텀 이동 모드 (CMOVE_Climbing)
 * 2. 선형 레일 기반 이동 (Start → End 직선 경로)
 * 3. 네트워크 예측 지원 (FSavedMove_CY)
 * 4. 입력 기반 상/하 이동
 * 동작 원리:
 * - BeginClimbLadder() 호출 시 MOVE_Custom 모드로 전환
 * - PhysLadder()에서 매 프레임 사다리 물리 처리
 * - LadderAttachSpot 파라미터로 사다리 상의 위치 추적 (0 ~ RailLength)
 * - 입력을 RailDir에 투영하여 상/하 이동 속도 계산
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHMEIFYOUCAN_API UCYCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCYCharacterMovementComponent();

	/**
	 * 사다리 등반 시작
	 * @param InLadder - 타고 있는 사다리 액터 (참조 유지용)
	 * @param InStart - 사다리 시작점 (월드 좌표, 하단)
	 * @param InEnd - 사다리 끝점 (월드 좌표, 상단)
	 * @param InFacing - 사다리가 향하는 방향 (캐릭터가 바라볼 방향)
	 * @param InAttachSpot - 초기 부착 위치 (0 ~ RailLength), -1이면 현재 위치에서 자동 계산
	 * 호출 시점: Ability나 RPC에서 사다리 상호작용 시작 시
	 * 효과: MOVE_Custom(CMOVE_Climbing) 모드로 전환, 중력 비활성화
	 */
	UFUNCTION(BlueprintCallable, Category="CY|Movement|Ladder")
	void BeginClimbLadder(AActor* InLadder, const FVector& InStart, const FVector& InEnd, const FVector& InFacing, float InAttachSpot = -1.f);

	/**
	 * 사다리 등반 종료
	 * @param bStepOffTop - true면 상단 탈출, false면 하단 탈출
	 * 호출 시점: 사다리 상/하단 도달 시 또는 강제 종료 시
	 * 효과: Walking 또는 Falling 모드로 복귀, 중력 복원
	 */
	UFUNCTION(BlueprintCallable, Category="CY|Movement|Ladder")
	void EndClimbLadder(bool bStepOffTop);

	/**
	 * 현재 사다리 타는 중인지 확인
	 */
	UFUNCTION(BlueprintPure, Category="CY|Movement|Ladder")
	bool IsClimbingLadder() const;

	/**
	 * 사다리 타는 중이면 MaxClimbSpeed 반환
	 * 그 외에는 부모 클래스의 기본 동작 사용
	 */
	virtual float GetMaxSpeed() const override;

	float GetLadderAttachSpot() const { return LadderAttachSpot; }
	void SetLadderAttachSpot(const float InAttachSpot) { LadderAttachSpot = InAttachSpot; }

protected:
	/**
	 * 이동 모드 변경 시 호출되는 콜백
	 */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/**
	 * 커스텀 이동 모드 물리 처리
	 */
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	/**
	 * 압축된 네트워크 플래그에서 상태 복원
	 * FLAG_Custom_0 비트를 사다리 타기 의도로 사용
	 * 클라이언트 예측 힌트용 (실제 모드 전환은 Ability/RPC에서 처리)
	 */
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	/**
	 * 클라이언트 예측 데이터 반환
	 * 커스텀 SavedMove(FSavedMove_CY)를 사용하기 위해
	 * FNetworkPredictionData_Client_CY 인스턴스 반환
	 */
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

private:
	/**
	 * 사다리 물리 처리 (매 프레임 호출)
	 *
	 * 동작 순서:
	 * 1. 입력 벡터를 RailDir에 투영하여 상/하 이동 속도 계산
	 * 2. LadderAttachSpoy 파라미터 업데이트 (적분)
	 * 3. 목표 위치/회전 계산 (레일 위치 - LadderFacing * StandOff)
	 * 4. MoveUpdatedComponent로 이동
	 * 5. Velocity 업데이트
	 * @param DeltaTime - 프레임 시간
	 * @param Iterations - 물리 반복 횟수
	 */
	void PhysLadder(float DeltaTime, int32 Iterations);

	/**
	 * 캐릭터를 사다리 레일에 스냅하고 방향 정렬
	 * 사다리 진입 시 즉시 호출되어 캐릭터를 정확한 위치/회전으로 이동
	 * ETeleportType::TeleportPhysics 사용하여 물리 충돌 무시
	 */
	void SnapToRailAndFace();

	/**
	 * 월드 위치(캐릭터 위치)를 사다리 레일 상의 파라미터 s로 투영
	 * @param WorldPos - 투영할 월드 좌표
	 * @return 사다리 레일 상의 거리 (0 ~ RailLength)
	 * 계산 방법: 내적을 이용한 선형 투영
	 * S = Dot(WorldPos - LadderStart, RailDir)
	 */
	float ProjectAttachSpot(const FVector& WorldPos) const;

public:
	/**
	 * 사다리 타기 의도 플래그
	 * - 네트워크 동기화에 사용
	 * - BeginClimbLadder()에서 true 설정
	 * - EndClimbLadder()에서 false 설정
	 */
	UPROPERTY()
	uint8 bWantsToClimb : 1;
	
private:

	/**
	 * 현재 타고 있는 사다리 액터 
	 * - Transient: 저장/로드 시 직렬화 제외
	 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> LadderActor;

	/**
	 * 사다리 시작점 (월드 좌표, 하단)
	 * BeginClimbLadder()에서 설정
	 */
	FVector LadderStart = FVector::ZeroVector;

	/**
	 * 사다리 끝점 (월드 좌표, 상단)
	 * BeginClimbLadder()에서 설정
	 */
	FVector LadderEnd = FVector::ZeroVector;

	/**
	 * 사다리가 향하는 방향 (월드 수평 방향)
	 * 캐릭터가 사다리를 바라보는 방향
	 */
	FVector CharToLadderFacing = FVector::ForwardVector;

	/**
	 * 사다리 레일 상의 현재 위치 파라미터
	 * 범위: 0 (하단) ~ RailLength (상단)
	 * PhysLadder()에서 매 프레임 업데이트
	 * 계산: LadderAttachSpot += SpeedAlongRail * DeltaTime
	 */
	float LadderAttachSpot = 0.f;

	/**
	 * 사다리 타기 최대 속도 (cm/s)
	 * 입력 방향에 따라 상/하 이동 속도 결정
	 */
	UPROPERTY(EditAnywhere, Category="CY|Movement|Ladder")
	float MaxClimbSpeed = 180.f;

	/**
	 * 사다리 평면에서 캐릭터를 떨어뜨리는 거리 (cm)
	 * 캐릭터가 사다리 메시와 겹치지 않도록 약간 앞으로 배치
	 */
	UPROPERTY(EditAnywhere, Category="CY|Movement|Ladder")
	float LadderStandOff = 30.f;

	/**
	 * 스냅 강도 (현재 미사용)
	 * TODO : 부드러운 스냅 보간에 사용 가능
	 */
	UPROPERTY(EditAnywhere, Category="CY|Movement|Ladder")
	float SnapStrength = 20.f;
	
	/**
	 * 사다리 레일 길이 (cm)
	 * BeginClimbLadder()에서 설정
	 */
	float RailLength = 0.f;

	/**
	 * 사다리 레일 방향 
	 * BeginClimbLadder()에서 설정
	 * 입력을 이 방향에 투영하여 상/하 이동 속도 계산
	 */
	FVector RailDirection = FVector::UpVector;

	/**
	 * 캐싱을 위한 변수들
	 */
	float DefaultGravityScale = 0.f;
	float DefaultBrakingFrictionFactor = 0.f;
	bool bSavedOrientRotationToMovement = false;
	bool bSavedUseControllerDesiredRotation = false;
};

/**
 * 커스텀 SavedMove 클래스
 * UE의 클라이언트 예측 시스템에서 사용하는 이동 데이터 저장 구조
 * 역할:
 * 1. 클라이언트가 서버에 전송할 이동 데이터 압축
 * 2. 서버 응답 후 재시뮬레이션 시 이동 복원
 * 3. 네트워크 대역폭 절약 (비트 플래그 사용)
 * 동작 흐름:
 * 1. SetMoveFor(): 현재 CMC 상태를 SavedMove에 저장
 * 2. GetCompressedFlags(): 상태를 비트 플래그로 압축
 * 3. 서버로 전송
 * 4. PrepMoveFor(): 재시뮬레이션 시 SavedMove에서 CMC로 복원
 */
class FSavedMove_CY : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	FSavedMove_CY();
	
	/**
	 * 사다리 타기 의도 플래그 (1비트)
	 * true: 클라이언트가 사다리 타는 중
	 * false: 일반 이동 중
	 * FLAG_Custom_0 비트로 네트워크 전송
	 */
	uint8 bWantsToClimb : 1;

	float SavedLadderAttachSpot = 0.f;

	/**
	 * SavedMove 초기화
	 * 재사용 전 모든 필드를 기본값으로 리셋
	 */
	virtual void Clear() override;

	/**
	 * 상태를 압축된 플래그로 변환
	 * @return 8비트 플래그 (FLAG_Custom_0 ~ FLAG_Custom_3 사용 가능)
	 * bWantsToClimb가 true면 FLAG_Custom_0 비트 설정
	 * 네트워크 대역폭 절약을 위한 핵심 최적화
	 */
	virtual uint8 GetCompressedFlags() const override;

	/**
	 * 이동 병합 가능 여부 체크
	 * @param NewMove - 새로운 이동 데이터
	 * @param InCharacter - 캐릭터
	 * @param MaxDelta - 최대 시간 차이
	 * @return true면 두 이동을 하나로 병합 가능
	 * 조건: bWantsToClimb 상태가 동일해야 병합 가능
	 * 사다리 타기 중/아닐 때 이동은 병합 불가
	 */
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

	/**
	 * 현재 CMC 상태를 SavedMove에 저장
	 * @param Character - 캐릭터
	 * @param InDeltaTime - 프레임 시간
	 * @param NewAccel - 가속도
	 * @param ClientData - 클라이언트 예측 데이터
	 * 호출 시점: 클라이언트가 서버에 이동 데이터 전송 전
	 * 동작: IsClimbingLadder() 결과를 bWantsToClimb에 저장
	 */
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

	/**
	 * SavedMove 데이터를 CMC로 복원
	 * @param Character - 캐릭터
	 * 호출 시점: 서버 응답 후 재시뮬레이션 직전
	 * 주의: 일반적으로 BeginClimbLadder는 Ability/RPC로 처리하므로
	 * 여기서 강제 모드 전환은 하지 않음 (예측 힌트로만 사용)
	 */
	virtual void PrepMoveFor(ACharacter* Character) override;
};

/**
 * 클라이언트 예측 데이터 팩토리
 * 역할:
 * - FSavedMove_CY 인스턴스를 생성하는 팩토리
 * - 이동 데이터 풀링 관리
 * UE의 클라이언트 예측 시스템에서 사용
 * GetPredictionData_Client()에서 반환
 */
class FNetworkPredictionData_Client_CY : public FNetworkPredictionData_Client_Character
{
public:

	FNetworkPredictionData_Client_CY(const UCharacterMovementComponent& ClientMovement);

	/**
	 * 새로운 SavedMove 할당
	 * @return FSavedMove_CY 인스턴스 (스마트 포인터)
	 * - 커스텀 SavedMove(FSavedMove_CY) 생성
	 * - UE의 이동 예측 시스템에서 자동으로 호출
	 * - 이 함수를 오버라이드하지 않으면 기본 FSavedMove_Character가 사용됨
	 * - 커스텀 필드(bWantsToClimb)를 사용하려면 반드시 오버라이드 필요
	 */
	virtual FSavedMovePtr AllocateNewMove() override;

};


