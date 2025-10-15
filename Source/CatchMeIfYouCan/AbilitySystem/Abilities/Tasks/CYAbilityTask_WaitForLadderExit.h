// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "CYAbilityTask_WaitForLadderExit.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLadderExitDelegate);

class UCYCharacterMovementComponent;

/**
 * 사다리 등반 상태 모니터링 및 이탈 감지 태스크
 * 주요 기능:
 * 1. 주기적으로 캐릭터의 사다리 위치 추적
 * 2. 상/하단 도달 및 입력 의도 감지
 * 3. 점프 입력 또는 비정상 상태 감지 시 취소
 * 이탈 조건:
 * - 상단 근처 + 상향 입력 → OnExitTop
 * - 하단 근처 + 하향 입력 → OnExitBottom
 * - 점프 입력 또는 사다리 무효화 → OnCancelled
 * 레일 파라미터 계산:
 * RailParameter = Dot(CharacterPos - BottomPos, ClimbDirection)
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYAbilityTask_WaitForLadderExit : public UAbilityTask
{
    GENERATED_BODY()

public:
    /**
     * @param OwningAbility - 소유 어빌리티
     * @param LadderActor - 감시할 사다리 액터
     * @param LadderBottomLocation - 사다리 하단 위치
     * @param LadderTopLocation - 사다리 상단 위치
     * @param LadderFacingDirection - 사다리 정면 방향
     * @param UpdateFrequency - 체크 주기 (초, 기본 0.02 = 50Hz)
     * @param BottomEdgeThreshold - 하단 이탈 임계값 (cm)
     * @param TopEdgeThreshold - 상단 이탈 임계값 (cm)
     * @param EntryTargetLocation - 진입 타겟 위치 (사다리 등반 시작 위치)
     * @param SafetyMargin - 진입 타겟 위치 안전 거리 마진(cm)
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true", DisplayName="Wait For Ladder Exit"))
    static UCYAbilityTask_WaitForLadderExit* CreateWaitForLadderExitTask(
        UGameplayAbility* OwningAbility,
        AActor* LadderActor,
        FVector LadderBottomLocation,
        FVector LadderTopLocation,
        FVector LadderFacingDirection,
        float UpdateFrequency = 0.02f,
        float BottomEdgeThreshold = 18.0f,
        float TopEdgeThreshold = 18.0f,
        FVector EntryTargetLocation = FVector::ZeroVector,
        float SafetyMargin = 20.0f);

protected:
 
    /** 태스크 활성화 - 타이머 시작 */
    virtual void Activate() override;


    /** 태스크 종료 - 타이머 정리 */
    virtual void OnDestroy(bool bInOwnerFinished) override;

private:

    /** 주기적으로 호출되는 메인 체크 함수 */
    void PerformExitConditionCheck();

    /** 점프 태그 이벤트 콜백 */
    void OnJumpTagChanged(const FGameplayTag Tag, int32 NewCount);
    
    /**
     * 캐릭터의 현재 레일 위치 계산
     * @param CharacterLocation - 캐릭터의 월드 위치
     * @return 레일 상의 위치 (0 = 하단, RailLength = 상단)
     */
    float CalculateCharacterRailPosition(const FVector& CharacterLocation) const;
    
    /**
     * 캐릭터의 입력 의도 분석
     * @param Character - 분석할 캐릭터
     * @return 1.0 = 상향, -1.0 = 하향, 0.0 = 중립
     */
    float AnalyzeClimbingIntent(const class ACharacter* Character) const;
    
    /**
     * 사다리 상/하단 근접 여부 체크
     * @param RailPosition - 현재 레일 위치
     * @param bCheckTop - true면 상단, false면 하단 체크
     * @return 근접했으면 true
     */
    bool IsNearLadderEdge(float RailPosition, bool bCheckTop) const;
    
    /**
     * 캐릭터와 사다리의 유효성 검증
     * @param OutCharacter - [출력] 유효한 캐릭터
     * @param OutMovementComponent - [출력] 유효한 MovementComponent
     * @return 모두 유효하면 true
     */
    bool ValidateCharacterAndLadder(ACharacter*& OutCharacter, UCYCharacterMovementComponent*& OutMovementComponent) const;

    /**
     * 캐릭터와 사다리 레일 간의 수평 거리 계산
     * @param CharacterLocation - 캐릭터의 현재 위치
     * @return 레일 중심선과의 최단 수평 거리 (cm)
     */
    float CalculateHorizontalDistanceFromRail(const FVector& CharacterLocation) const;

public:
    /** 사다리 상단으로 이탈 시 브로드캐스트 */
    UPROPERTY(BlueprintAssignable, Category="Ladder Events")
    FOnLadderExitDelegate OnExitTop;
    
    /** 사다리 하단으로 이탈 시 브로드캐스트 */
    UPROPERTY(BlueprintAssignable, Category="Ladder Events")
    FOnLadderExitDelegate OnExitBottom;
    
    /** 사다리 등반 취소 시 브로드캐스트 */
    UPROPERTY(BlueprintAssignable, Category="Ladder Events")
    FOnLadderExitDelegate OnCancelled;
    
private:
    /** 감시 중인 사다리 액터 (약참조) */
    UPROPERTY()
    TWeakObjectPtr<AActor> MonitoredLadder;
    
    /** 사다리 하단 월드 위치 */
    FVector LadderBottomWorldLocation = FVector::ZeroVector;
    
    /** 사다리 상단 월드 위치 */
    FVector LadderTopWorldLocation = FVector::ZeroVector;
    
    /** 사다리 정면 방향 (수평 정규화) */
    FVector LadderFacingDirection = FVector::ForwardVector;
    
    /** 사다리 등반 방향 (Bottom → Top) */
    FVector LadderClimbDirection = FVector::UpVector;
    
    /** 사다리 전체 길이 (cm) */
    float LadderTotalHeight = 0.0f;

    /** 체크 주기 (초) */
    float CheckUpdateFrequency = 0.02f;
    
    /**  하단 이탈 임계값 */
    float BottomEdgeProximityThreshold = 18.0f;
    
    /** 상단 이탈 임계값 */
    float TopEdgeProximityThreshold = 18.0f;

    /** 진입 시 계산된 목표 위치 (거리 기준점) */
    FVector EntryTargetLocation = FVector::ZeroVector;

    /** 최대 수평 거리 (cm) */
    float HorizontalDistanceSafetyMargin = 20.0f;

    /** 최대 허용 수평 거리 (한 번만 계산, 캐싱) */
    float MaxAllowedHorizontalDistance = 0.0f;
    
    /** 입력 의도 판단 임계값 (내적값) */
    constexpr static float INPUT_INTENT_THRESHOLD = 0.2f;

    /** 주기적 체크 타이머 핸들 */
    FTimerHandle PeriodicCheckTimer;

    /** 점프 태그 델리게이트 핸들 */
    FDelegateHandle JumpTagDelegateHandle;
};