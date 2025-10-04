// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CYLadderBase.generated.h"

class UBoxComponent;
class UArrowComponent;

/**
 * 직선형 사다리 베이스 액터
 * 주요 개념:
 * - Rail: 사다리의 가상 직선 경로 (Start → End)
 * - Rail Parameter: 레일 상의 위치를 나타내는 스칼라값 (0 ~ RailLength)
 * - Facing Direction: 사다리를 오를 때 캐릭터가 바라보는 방향 (수평 성분만)
 * - Stand-off Distance: 캐릭터가 사다리 메시로부터 떨어져 있는 거리
 * 컴포넌트 구조:
 * Root
 * ├── Start (사다리 하단 위치)
 * ├── End (사다리 상단 위치)
 * ├── Facing (사다리 정면 방향 표시)
 * └── UseVolume (상호작용 감지 영역)
 */
UCLASS(Blueprintable)
class CATCHMEIFYOUCAN_API ACYLadderBase : public AActor
{
    GENERATED_BODY()

public:
    ACYLadderBase();

    /** 사다리 하단의 월드 좌표 반환 */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Properties")
    FVector GetBottomWorldLocation() const;

    /** 사다리 상단의 월드 좌표 반환 */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Properties")
    FVector GetTopWorldLocation() const;

    /** 사다리 정면 방향 벡터 반환 (수평 평면에 투영된 정규화 벡터) */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Properties")
    FVector GetHorizontalFacingDirection() const;

    /** 사다리 레일의 방향 벡터 반환 (Bottom → Top 정규화 벡터) */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Properties")
    FVector GetClimbingDirection() const;

    /** 사다리의 전체 길이 반환 (cm) */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Properties")
    float GetTotalHeight() const;

    /**
     * 월드 좌표를 사다리 레일 상의 파라미터로 변환
     * @param WorldLocation - 변환할 월드 좌표
     * @return 레일 파라미터 (0 = 하단, TotalHeight = 상단)
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Calculations")
    float ConvertWorldLocationToRailParameter(const FVector& WorldLocation) const;

    /**
     * 레일 파라미터에 해당하는 캐릭터 Transform 계산
     * @param RailParameter - 레일 상의 위치 (0 ~ TotalHeight)
     * @param CharacterOffsetFromLadder - 사다리로부터 캐릭터를 떨어뜨릴 거리
     * @return 캐릭터가 위치해야 할 Transform
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Calculations")
    FTransform CalculateCharacterTransformAtParameter(float RailParameter, float CharacterOffsetFromLadder = 30.0f) const;

    /**
     * 사다리 진입 시 초기 위치 계산
     * @param bIsClimbingUp - true면 하단에서 시작, false면 상단에서 시작
     * @param EdgeOffset - 가장자리로부터의 오프셋 거리
     * @return 진입 시작 레일 파라미터
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Calculations")
    float CalculateEntryRailParameter(bool bIsClimbingUp, float EdgeOffset = 25.0f) const;

    /**
     * 특정 액터가 이 사다리를 사용할 수 있는지 검증
     * @param Character - 검증할 캐릭터
     * @param MaxHorizontalDistance - 허용 최대 수평 거리
     * @param MaxAngleDegrees - 허용 최대 각도 (도 단위)
     * @return 사용 가능하면 true
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Validation")
    bool CanCharacterUseLadder(const AActor* Character, float MaxHorizontalDistance = 120.0f, float MaxAngleDegrees = 55.0f) const;

    /** 상호작용 감지 박스 컴포넌트 반환 */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Components")
    UBoxComponent* GetInteractionVolume() const { return InteractionVolume; }

    /** 에디터에서 사다리 경로 및 방향 시각화 */
    virtual void OnConstruction(const FTransform& Transform) override;

protected:

    /** 루트 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    /** 사다리 하단 위치 마커 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(DisplayName="Bottom Point"))
    TObjectPtr<USceneComponent> BottomPoint;

    /** 사다리 상단 위치 마커 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(DisplayName="Top Point"))
    TObjectPtr<USceneComponent> TopPoint;

    /** 사다리 정면 방향 표시 화살표 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(DisplayName="Facing Direction"))
    TObjectPtr<UArrowComponent> FacingArrow;

    /** 상호작용 가능 영역 박스 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(DisplayName="Interaction Volume"))
    TObjectPtr<UBoxComponent> InteractionVolume;

    /** 에디터에서 디버그 시각화 활성화 */
    UPROPERTY(EditAnywhere, Category="Debug", meta=(DisplayName="Show Debug Visualization"))
    bool bShowDebugVisualization = true;

    /** 디버그 선 표시 시간 (0 = 영구) */
    UPROPERTY(EditAnywhere, Category="Debug", meta=(DisplayName="Debug Line Duration"))
    float DebugLineDuration = 0.0f;

private:
    /** 내부 헬퍼: 수직 성분을 제거한 정규화 벡터 반환 */
    FVector GetHorizontalNormalizedVector(const FVector& Vector) const;
};