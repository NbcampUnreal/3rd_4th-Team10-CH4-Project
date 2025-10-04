// Fill out your copyright notice in the Description page of Project Settings.


#include "CYLadderBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"

ACYLadderBase::ACYLadderBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 루트 컴포넌트 생성
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(RootSceneComponent);

    // 하단 위치 마커
    BottomPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BottomPoint"));
    BottomPoint->SetupAttachment(RootSceneComponent);
    
    // 상단 위치 마커  
    TopPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopPoint"));
    TopPoint->SetupAttachment(RootSceneComponent);
    
    // 정면 방향 화살표
    FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingDirection"));
    FacingArrow->SetupAttachment(RootSceneComponent);
    FacingArrow->ArrowSize = 1.0f;
    FacingArrow->SetHiddenInGame(true);
    
    // 상호작용 영역 박스
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootSceneComponent);
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECC_WorldStatic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

FVector ACYLadderBase::GetBottomWorldLocation() const
{
    return BottomPoint ? BottomPoint->GetComponentLocation() : GetActorLocation();
}

FVector ACYLadderBase::GetTopWorldLocation() const
{
    return TopPoint ? TopPoint->GetComponentLocation() : GetActorLocation() + FVector(0, 0, 200);
}

FVector ACYLadderBase::GetHorizontalFacingDirection() const
{
    // Arrow 컴포넌트의 전방 벡터 가져오기
    FVector FacingVector = FacingArrow ? FacingArrow->GetForwardVector() : GetActorForwardVector();
    
    // 수평 평면에 투영하여 정규화
    return GetHorizontalNormalizedVector(FacingVector);
}

FVector ACYLadderBase::GetClimbingDirection() const
{
    // 하단에서 상단으로의 정규화된 방향 벡터
    const FVector DirectionVector = GetTopWorldLocation() - GetBottomWorldLocation();
    return DirectionVector.GetSafeNormal();
}

float ACYLadderBase::GetTotalHeight() const
{
    // 사다리의 총 높이 (cm)
    return FVector::Distance(GetBottomWorldLocation(), GetTopWorldLocation());
}

float ACYLadderBase::ConvertWorldLocationToRailParameter(const FVector& WorldLocation) const
{
    const FVector BottomLocation = GetBottomWorldLocation();
    const FVector ClimbDirection = GetClimbingDirection();
    const float TotalHeight = GetTotalHeight();
    
    // 예외 처리: 높이가 0인 경우
    if (TotalHeight <= KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    // 월드 좌표를 레일에 투영
    const FVector LocationRelativeToBottom = WorldLocation - BottomLocation;
    const float ProjectedDistance = FVector::DotProduct(LocationRelativeToBottom, ClimbDirection);
    
    // 0 ~ TotalHeight 범위로 클램프
    return FMath::Clamp(ProjectedDistance, 0.0f, TotalHeight);
}

FTransform ACYLadderBase::CalculateCharacterTransformAtParameter(float RailParameter, float CharacterOffsetFromLadder) const
{
    const FVector BottomLocation = GetBottomWorldLocation();
    const FVector ClimbDirection = GetClimbingDirection();
    const FVector FacingDirection = GetHorizontalFacingDirection();
    
    // 레일 상의 위치 계산
    const float ClampedParameter = FMath::Clamp(RailParameter, 0.0f, GetTotalHeight());
    const FVector RailPosition = BottomLocation + ClimbDirection * ClampedParameter;
    
    // 사다리로부터 캐릭터를 떨어뜨림 (정면 반대 방향)
    const FVector CharacterPosition = RailPosition - FacingDirection * CharacterOffsetFromLadder;
    
    // 캐릭터 회전: X축은 정면, Z축은 등반 방향
    const FQuat CharacterRotation = FRotationMatrix::MakeFromXZ(FacingDirection, ClimbDirection).ToQuat();
    
    return FTransform(CharacterRotation, CharacterPosition);
}

float ACYLadderBase::CalculateEntryRailParameter(bool bIsClimbingUp, float EdgeOffset) const
{
    const float Height = GetTotalHeight();
    
    if (Height <= KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }
    
    // 상향 등반: 하단에서 시작
    // 하향 등반: 상단에서 시작
    if (bIsClimbingUp)
    {
        return FMath::Clamp(EdgeOffset, 0.0f, Height);
    }
    else
    {
        return FMath::Clamp(Height - EdgeOffset, 0.0f, Height);
    }
}

bool ACYLadderBase::CanCharacterUseLadder(const AActor* Character, float MaxHorizontalDistance, float MaxAngleDegrees) const
{
    if (!Character)
    {
        return false;
    }
    
    // 1. 수평 거리 체크
    const float HorizontalDistance = FVector::Dist2D(Character->GetActorLocation(), GetActorLocation());
    if (HorizontalDistance > MaxHorizontalDistance)
    {
        return false;
    }
    
    // 2. 각도 체크 (캐릭터가 사다리를 바라보고 있는지)
    const FVector CharacterForward = GetHorizontalNormalizedVector(Character->GetActorForwardVector());
    const FVector LadderFacing = GetHorizontalFacingDirection();
    
    const float DotProduct = FVector::DotProduct(CharacterForward, LadderFacing);
    const float AngleDegrees = UKismetMathLibrary::DegAcos(DotProduct);
    
    return AngleDegrees <= MaxAngleDegrees;
}

void ACYLadderBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
    if (!bShowDebugVisualization || !GetWorld())
    {
        return;
    }
    
    const FVector Bottom = GetBottomWorldLocation();
    const FVector Top = GetTopWorldLocation();
    const FVector Facing = GetHorizontalFacingDirection();
    
    // 사다리 레일 라인 (청록색)
    DrawDebugLine(GetWorld(), Bottom, Top, FColor::Cyan, false, DebugLineDuration, 0, 2.0f);
    
    // 정면 방향 화살표 (녹색)
    const FVector ArrowStart = GetActorLocation();
    const FVector ArrowEnd = ArrowStart + Facing * 100.0f;
    DrawDebugDirectionalArrow(GetWorld(), ArrowStart, ArrowEnd, 20.0f, FColor::Green, false, DebugLineDuration, 0, 2.0f);
    
    // 상/하단 포인트 표시 (노란색)
    DrawDebugSphere(GetWorld(), Bottom, 10.0f, 8, FColor::Yellow, false, DebugLineDuration);
    DrawDebugSphere(GetWorld(), Top, 10.0f, 8, FColor::Yellow, false, DebugLineDuration);
}

FVector ACYLadderBase::GetHorizontalNormalizedVector(const FVector& Vector) const
{
    // 수직 성분 제거
    const FVector VerticalComponent = FVector::DotProduct(Vector, FVector::UpVector) * FVector::UpVector;
    const FVector HorizontalVector = Vector - VerticalComponent;
    
    // 정규화 (0벡터 방지)
    const FVector NormalizedVector = HorizontalVector.GetSafeNormal();
    return NormalizedVector.IsNearlyZero() ? FVector::ForwardVector : NormalizedVector;
}
