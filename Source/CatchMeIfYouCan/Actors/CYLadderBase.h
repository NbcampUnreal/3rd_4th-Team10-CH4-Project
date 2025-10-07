// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/CYInteractionInfo.h"
#include "Interaction/CYWorldInteractable.h"
#include "CYLadderBase.generated.h"

class UBoxComponent;
class UArrowComponent;

UENUM(BlueprintType)
enum class ELadderEntryType : uint8
{
    None    UMETA(DisplayName = "None"),
    Bottom  UMETA(DisplayName = "Bottom Entry"),
    Middle  UMETA(DisplayName = "Middle Entry"),  
    Top     UMETA(DisplayName = "Top Entry")
};

/**
 * 상호작용 가능한 사다리 액터
 * - 상/하단: E키 상호작용으로 진입
 * - 중간: 점프 중 자동 그랩 (조건 충족시)
 * - MovementComponent와 연동하여 사다리 물리 처리
 */
UCLASS(Blueprintable)
class CATCHMEIFYOUCAN_API ACYLadderBase : public ACYWorldInteractable
{
    GENERATED_BODY()

public:
    ACYLadderBase();

    UFUNCTION(BlueprintPure, Category="CY|Ladder")
    FVector GetBottomWorldLocation() const;

    UFUNCTION(BlueprintPure, Category="CY|Ladder")
    FVector GetTopWorldLocation() const;

    UFUNCTION(BlueprintPure, Category="CY|Ladder")
    FVector GetHorizontalFacingDirection() const;

    UFUNCTION(BlueprintPure, Category="CY|Ladder")
    FVector GetClimbingDirection() const;

    UFUNCTION(BlueprintPure, Category="CY|Ladder")
    float GetTotalHeight() const;

    /**
     * 플레이어의 현재 진입 타입 반환
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Interaction")
    ELadderEntryType GetPlayerEntryType(const AActor* Player) const;

    /**
     * 진입 타입과 캐릭터 위치에 따른 초기 레일 위치 계산
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Interaction")
    float CalculateInitialRailParameter(ELadderEntryType EntryType, const ACharacter* Character, float EdgeOffset = 25.0f) const;

    /**
     * 진입 방향 결정 (상향/하향)
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|Interaction")
    void DetermineClimbDirection(const ELadderEntryType EntryType, const ACharacter* Character, bool& OutIsClimbingUp) const;

    /**
     * 중간 영역에서 자동 그랩 가능 여부 체크
     */
    UFUNCTION(BlueprintPure, Category="CY|Ladder|AutoGrab")
    bool CanAutoGrabFromMiddle(const ACharacter* Character) const;
    
    virtual FCYInteractionInfo GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const override;
    virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;
    virtual bool CanInteraction(const FCYInteractionQuery& InteractionQuery) const override;

protected:
    
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    // TODO : 추후 블루 프린트나 맵을 참고해서 레벨에 배치된 엑터에 대해 직접 조정
    void SetupEntryBoxes();

    UFUNCTION()
    void OnEntryBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEntryBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    /** 플레이어 진입 타입 업데이트 */
    void UpdatePlayerEntryType(AActor* Player);
    
    /** 중간 박스에서 자동 그랩 시도 */
    void TryAutoGrabLadder(ACharacter* Character);
    
    /** 자동 그랩시 등반 방향 결정 */
    void DetermineClimbDirectionForAutoGrab(const ACharacter* Character, bool& OutIsClimbingUp) const;

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    /** 사다리 하단 위치 (MovementComponent에서 필요) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USceneComponent> BottomPoint;

    /** 사다리 상단 위치 (MovementComponent에서 필요) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USceneComponent> TopPoint;

    /** 사다리 정면 방향 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UArrowComponent> FacingArrow;

    /** 상단 진입 박스 (E키 상호작용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UBoxComponent> TopEntryBox;

    /** 중간 진입 박스 (자동 그랩) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UBoxComponent> MiddleEntryBox;

    /** 하단 진입 박스 (E키 상호작용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UBoxComponent> BottomEntryBox;

    /** 시각적 메시 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> LadderMesh;

    /** 진입 박스 크기 */
    UPROPERTY(EditAnywhere, Category="CY|Ladder", meta=(ClampMin="50.0", ClampMax="300.0"))
    float EntryBoxRadius = 150.0f;

    /** 상/하단 박스 높이 비율 */
    UPROPERTY(EditAnywhere, Category="CY|Ladder", meta=(ClampMin="0.1", ClampMax="0.4"))
    float EdgeBoxHeightRatio = 0.25f;

    /** 자동 그랩 활성화 여부 */
    UPROPERTY(EditAnywhere, Category="CY|Ladder|AutoGrab")
    bool bEnableAutoGrab = true;

    /** 자동 그랩을 위한 최소 수직 속도 */
    UPROPERTY(EditAnywhere, Category="CY|Ladder|AutoGrab", meta=(ClampMin="0", ClampMax="2000"))
    float MinVerticalSpeedForAutoGrab = 50.0f;

    /** 자동 그랩시 최대 각도 */
    UPROPERTY(EditAnywhere, Category="CY|Ladder|AutoGrab", meta=(ClampMin="0", ClampMax="180"))
    float MaxAutoGrabAngle = 90.0f;

    /** 자동 그랩시 최대 거리 비율 */
    UPROPERTY(EditAnywhere, Category="CY|Ladder|AutoGrab", meta=(ClampMin="0.3", ClampMax="1.0"))
    float AutoGrabDistanceRatio = 0.7f;

    /** 상호작용 정보 */
    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder|Interaction")
    FCYInteractionInfo ClimbUpInteractionInfo;

    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder|Interaction")
    FCYInteractionInfo ClimbDownInteractionInfo;

    UPROPERTY(EditDefaultsOnly, Category="CY|Ladder|Interaction")
    FCYInteractionInfo ClimbMiddleInteractionInfo;

    /** 플레이어별 진입 타입 */
    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, ELadderEntryType> PlayerEntryTypeMap;

    UPROPERTY(EditAnywhere, Category="Debug")
    bool bShowDebugVisualization = true;
};