// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CYGameplayAbility.h"
#include "CYGameplayAbility_ClimbLadder_Enter.generated.h"


class ACYLadderBase;
class UCYCharacterMovementComponent;
class UCYAbilityTask_WaitForLadderExit;

/**
 * 사다리 등반 진입 및 관리 어빌리티
 * - 상호작용 시스템 또는 자동 그랩으로 활성화
 * - Movement Mode 전환 및 이탈 조건 모니터링
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYGameplayAbility_ClimbLadder_Enter : public UCYGameplayAbility
{
    GENERATED_BODY()
    
public:
    UCYGameplayAbility_ClimbLadder_Enter();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    /** 사다리 상단으로 이탈 */
    UFUNCTION()
    void HandleLadderExitFromTop();

    /** 사다리 하단으로 이탈 */
    UFUNCTION()
    void HandleLadderExitFromBottom();

    /** 사다리 등반 취소 (점프 등) */
    UFUNCTION()
    void HandleLadderClimbingCancelled();
    
private:
    /**
     * 이벤트 데이터에서 사다리 정보 추출 및 검증
     */
    bool CanExtractLadderInfo(const FGameplayEventData* TriggerEventData, ACYLadderBase*& OutLadder, FVector& OutBottomLocation, FVector& OutTopLocation, FVector& OutFacingDirection, float& OutLadderStandOff, float& OutInitialRailParameter, bool& OutIsClimbingUp) const;

    /** 하단 이탈 안전 마진 */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder|Exit", meta=(ClampMin="0.0", ClampMax="100.0"))
    float BottomExitSafetyMargin = 10.0f;  // BottomEntrySafetyMargin과 동일

    /** 상단 이탈 안전 마진 */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder|Exit", meta=(ClampMin="0.0", ClampMax="100.0"))
    float TopExitSafetyMargin = 10.0f;

    /** 캐싱된 MovementComponent */
    UPROPERTY(Transient)
    TObjectPtr<UCYCharacterMovementComponent> CachedMovementComponent;

    /** 현재 등반 중인 사다리 */
    UPROPERTY(Transient)
    TObjectPtr<ACYLadderBase> CurrentLadder;

    /** 이탈 모니터링 태스크 */
    UPROPERTY(Transient)
    TObjectPtr<UCYAbilityTask_WaitForLadderExit> ExitMonitorTask;
};
