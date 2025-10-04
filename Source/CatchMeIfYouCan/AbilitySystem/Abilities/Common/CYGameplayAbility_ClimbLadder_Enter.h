// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "CYGameplayAbility_ClimbLadder_Enter.generated.h"

class UCYCharacterMovementComponent;
class ACYLadderBase;

/**
 * 사다리 등반 진입 어빌리티
 * 주요 기능:
 * 1. 사용 가능한 사다리 탐색 및 검증
 * 2. 사다리 등반 모드로 전환 (MovementComponent)
 * 3. 사다리 이탈 조건 모니터링
 * 동작 흐름:
 * - 활성화 → 사다리 찾기 → 유효성 검증 → 등반 시작 → 이탈 감시
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_ClimbLadder_Enter : public UCYGameplayAbility
{
    GENERATED_BODY()
    
public:
    UCYGameplayAbility_ClimbLadder_Enter();

protected:

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    /** 사다리 상단으로 이탈 시 호출 */
    UFUNCTION()
    void HandleLadderExitFromTop();

    /** 사다리 하단으로 이탈 시 호출 */
    UFUNCTION()
    void HandleLadderExitFromBottom();

    /** 사다리 등반 취소 시 호출 (점프, 상태 이상 등) */
    UFUNCTION()
    void HandleLadderClimbingCancelled();
    
private:

    /**
     * 사용 가능한 사다리를 찾고 등반 파라미터를 결정
     * @param OutLadderActor - [출력] 찾은 사다리 액터
     * @param OutBottomLocation - [출력] 사다리 하단 위치
     * @param OutTopLocation - [출력] 사다리 상단 위치
     * @param OutFacingDirection - [출력] 사다리 정면 방향
     * @param OutInitialRailParameter - [출력] 초기 레일 위치
     * @param OutIsClimbingUp - [출력] 상향 등반 여부
     * @param EventData - 게임플레이 이벤트 데이터 (상호작용 시스템)
     * @return 유효한 사다리를 찾았으면 true
     */
    bool FindAndValidateLadder(ACYLadderBase*& OutLadderActor,FVector& OutBottomLocation,FVector& OutTopLocation, FVector& OutFacingDirection, float& OutInitialRailParameter, bool& OutIsClimbingUp, const FGameplayEventData* EventData) const;

    /**
     * 캐릭터 주변에서 가장 가까운 사다리 액터 탐색
     * @param SearchOrigin - 탐색 중심 위치
     * @return 가장 가까운 사다리 액터 (없으면 nullptr)
     */
    ACYLadderBase* FindNearestLadderInRange(const FVector& SearchOrigin) const;

    /**
     * 캐릭터의 이동 입력 방향으로 등반 방향 결정
     * @return true면 상향 등반, false면 하향 등반
     */
    bool DetermineClimbingDirection(const ACharacter* Character, const ACYLadderBase* Ladder) const;

    /** 사다리 진입 가능 최대 거리 (cm) */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder", meta=(DisplayName="Maximum Entry Distance"))
    float MaximumEntryDistance = 120.0f;

    /** 사다리 진입 가능 최대 각도 (도) */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder", meta=(DisplayName="Maximum Entry Angle"))
    float MaximumEntryAngleDegrees = 55.0f;

    /** 사다리 상/하단 진입 시 가장자리로부터의 오프셋 (cm) */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder", meta=(DisplayName="Edge Entry Offset"))
    float EdgeEntryOffset = 25.0f;

    /** 주변 사다리 검색 반경 (cm) */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder", meta=(DisplayName="Ladder Search Radius"))
    float LadderSearchRadius = 200.0f;

    /** 사다리 검색용 충돌 채널 */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder", meta=(DisplayName="Ladder Detection Channel"))
    TEnumAsByte<ECollisionChannel> LadderDetectionChannel = ECC_WorldStatic;

    /** 캐릭터의 커스텀 이동 컴포넌트 (런타임 캐시) */
    UPROPERTY(Transient)
    TObjectPtr<UCYCharacterMovementComponent> CharacterMovementComponent;
};
