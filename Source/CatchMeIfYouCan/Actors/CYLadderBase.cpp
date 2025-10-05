// Fill out your copyright notice in the Description page of Project Settings.


#include "CYLadderBase.h"

#include "AbilitySystemComponent.h"
#include "CYLogChannels.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/Abilities/CYAbilityGameplayTags.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Physics/CYCollisionChannels.h"

ACYLadderBase::ACYLadderBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // 루트
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(RootSceneComponent);

    // 위치 마커 (MovementComponent에서 사용)
    BottomPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BottomPoint"));
    BottomPoint->SetupAttachment(RootSceneComponent);
    
    TopPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopPoint"));
    TopPoint->SetupAttachment(RootSceneComponent);
    
    // 방향
    FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingDirection"));
    FacingArrow->SetupAttachment(RootSceneComponent);
    FacingArrow->SetHiddenInGame(true);

    // 진입 박스들
    TopEntryBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TopEntryBox"));
    TopEntryBox->SetupAttachment(RootSceneComponent);
    TopEntryBox->SetCollisionProfileName(TEXT("Interactable"));
    
    MiddleEntryBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MiddleEntryBox"));
    MiddleEntryBox->SetupAttachment(RootSceneComponent);
    MiddleEntryBox->SetCollisionProfileName(TEXT("Interactable"));
    
    BottomEntryBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BottomEntryBox"));
    BottomEntryBox->SetupAttachment(RootSceneComponent);
    BottomEntryBox->SetCollisionProfileName(TEXT("Interactable"));

    LadderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LadderMesh"));
    LadderMesh->SetupAttachment(RootSceneComponent);
    LadderMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    LadderMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    LadderMesh->SetCollisionResponseToChannel(CY_TraceChannel_Interaction, ECR_Block);
}

void ACYLadderBase::BeginPlay()
{
    Super::BeginPlay();

    // TODO : 추후 블루 프린트나 맵을 참고해서 레벨에 배치된 엑터에 대해 직접 조정
    SetupEntryBoxes();

    if (TopEntryBox)
    {
        TopEntryBox->OnComponentBeginOverlap.AddDynamic(this, &ACYLadderBase::OnEntryBoxBeginOverlap);
        TopEntryBox->OnComponentEndOverlap.AddDynamic(this, &ACYLadderBase::OnEntryBoxEndOverlap);
    }
    
    if (MiddleEntryBox)
    {
        MiddleEntryBox->OnComponentBeginOverlap.AddDynamic(this, &ACYLadderBase::OnEntryBoxBeginOverlap);
        MiddleEntryBox->OnComponentEndOverlap.AddDynamic(this, &ACYLadderBase::OnEntryBoxEndOverlap);
    }
    
    if (BottomEntryBox)
    {
        BottomEntryBox->OnComponentBeginOverlap.AddDynamic(this, &ACYLadderBase::OnEntryBoxBeginOverlap);
        BottomEntryBox->OnComponentEndOverlap.AddDynamic(this, &ACYLadderBase::OnEntryBoxEndOverlap);
    }
}

void ACYLadderBase::SetupEntryBoxes()
{
    const float Height = GetTotalHeight();
    const FVector Bottom = GetBottomWorldLocation();
    const FVector Top = GetTopWorldLocation();
    const FVector Center = (Bottom + Top) * 0.5f;
    
    const float EdgeBoxHeight = Height * EdgeBoxHeightRatio;
    const float MiddleBoxHeight = Height - (EdgeBoxHeight * 2);
    
    // 상단 박스
    if (TopEntryBox)
    {
        TopEntryBox->SetBoxExtent(FVector(EntryBoxRadius, EntryBoxRadius, EdgeBoxHeight * 0.5f));
        TopEntryBox->SetWorldLocation(Top - FVector(0, 0, EdgeBoxHeight * 0.5f));
    }
    
    // 중간 박스
    if (MiddleEntryBox && MiddleBoxHeight > 0)
    {
        MiddleEntryBox->SetBoxExtent(FVector(EntryBoxRadius, EntryBoxRadius, MiddleBoxHeight * 0.5f));
        MiddleEntryBox->SetWorldLocation(Center);
    }
    
    // 하단 박스
    if (BottomEntryBox)
    {
        BottomEntryBox->SetBoxExtent(FVector(EntryBoxRadius, EntryBoxRadius, EdgeBoxHeight * 0.5f));
        BottomEntryBox->SetWorldLocation(Bottom + FVector(0, 0, EdgeBoxHeight * 0.5f));
    }
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
    if (!FacingArrow) return FVector::ForwardVector;
    
    FVector Facing = FacingArrow->GetForwardVector();
    Facing.Z = 0;
    return Facing.GetSafeNormal();
}

FVector ACYLadderBase::GetClimbingDirection() const
{
    return (GetTopWorldLocation() - GetBottomWorldLocation()).GetSafeNormal();
}

float ACYLadderBase::GetTotalHeight() const
{
    return FVector::Distance(GetBottomWorldLocation(), GetTopWorldLocation());
}

ELadderEntryType ACYLadderBase::GetPlayerEntryType(const AActor* Player) const
{
    if (const ELadderEntryType* EntryType = PlayerEntryTypeMap.Find(Player))
    {
        return *EntryType;
    }
    return ELadderEntryType::None;
}

float ACYLadderBase::CalculateInitialRailParameter(ELadderEntryType EntryType, 
                                                   const ACharacter* Character,
                                                   float EdgeOffset) const
{
    const float Height = GetTotalHeight();
    
    switch (EntryType)
    {
        case ELadderEntryType::Top:
            return Height - EdgeOffset;
            
        case ELadderEntryType::Bottom:
            return EdgeOffset;
            
        case ELadderEntryType::Middle:
        {
            if (Character)
            {
                const FVector CharPos = Character->GetActorLocation();
                const FVector Bottom = GetBottomWorldLocation();
                const FVector ClimbDir = GetClimbingDirection();
                
                const FVector RelativePos = CharPos - Bottom;
                float ProjectedHeight = FVector::DotProduct(RelativePos, ClimbDir);
                
                return FMath::Clamp(ProjectedHeight, EdgeOffset, Height - EdgeOffset);
            }
            return Height * 0.5f;
        }
            
        default:
            return 0.0f;
    }
}

bool ACYLadderBase::DetermineClimbDirection(ELadderEntryType EntryType, const ACharacter* Character) const
{
    switch (EntryType)
    {
        case ELadderEntryType::Top:
            return false; // 항상 하향
            
        case ELadderEntryType::Bottom:
            return true;  // 항상 상향
            
        case ELadderEntryType::Middle:
            return DetermineClimbDirectionForAutoGrab(Character);
            
        default:
            return true;
    }
}

bool ACYLadderBase::DetermineClimbDirectionForAutoGrab(const ACharacter* Character) const
{
    if (!Character)
        return true;

    // 1. 수직 속도 기반
    const float VerticalVelocity = Character->GetVelocity().Z;
    if (FMath::Abs(VerticalVelocity) > 50.0f)
    {
        return VerticalVelocity > 0;
    }
    
    // 2. 입력 방향 기반
    const FVector InputDir = Character->GetLastMovementInputVector().GetSafeNormal2D();
    if (InputDir.SizeSquared() > 0.1f)
    {
        const FVector CharFacingOnLadder = -GetHorizontalFacingDirection();
        const float Dot = FVector::DotProduct(InputDir, CharFacingOnLadder);
        return Dot >= 0.0f;
    }
    
    // 3. 캐릭터 높이 기반
    const float CharHeight = Character->GetActorLocation().Z;
    const float LadderCenter = (GetBottomWorldLocation().Z + GetTopWorldLocation().Z) * 0.5f;
    return CharHeight < LadderCenter;
}

bool ACYLadderBase::CanAutoGrabFromMiddle(const ACharacter* Character) const
{
    if (!Character || !bEnableAutoGrab)
    {
        return false;
    }
    const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    if (!Movement)
    {
        return false;
    }
    
    // 1. 공중에 있어야 함
    if (!Movement->IsFalling())
    {
        return false;
    }
    
    // 2. TODO : 점프 말고 낙하시에도 가능하도록 고려
    if (Movement->Velocity.Z > MinFallingSpeedForAutoGrab)
    {
        return false;
    }
    
    // 3. 사다리를 바라보고 있어야 함
    const FVector CharForward = Character->GetActorForwardVector();
    const FVector ToLadder = (GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
    const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CharForward, ToLadder)));
    if (Angle > MaxAutoGrabAngle)
    {
        return false;
    }
    
    // 4. 충분히 가까워야 함
    const float Distance = FVector::Dist2D(Character->GetActorLocation(), GetActorLocation());
    if (Distance > EntryBoxRadius * AutoGrabDistanceRatio)
    {
        return false;
    }
    
    return true;
}

void ACYLadderBase::TryAutoGrabLadder(ACharacter* Character)
{
    if (!Character || !CanAutoGrabFromMiddle(Character))
    {
        UpdatePlayerEntryType(Character);
        return;
    }

    if (UAbilitySystemComponent* ASC = Character->GetComponentByClass<UAbilitySystemComponent>())
    {
        FGameplayEventData EventData;
        EventData.Instigator = Character;
        EventData.Target = this;  // 사다리 액터 전달
        
        // 어빌리티에서 직접 조회하도록 단순화
        ASC->HandleGameplayEvent(CYGameplayTags::Ability_Action_Climbing, &EventData);
        
        UE_LOG(LogCY, Warning, TEXT("Auto-grabbed ladder!"));
    }
}

void ACYLadderBase::OnEntryBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character)
    {
        return;
    }
    // 중간 박스 특별 처리
    if (OverlappedComponent == MiddleEntryBox)
    {
        if (bEnableAutoGrab)
        {
            TryAutoGrabLadder(Character);
        }
        else
        {
            UpdatePlayerEntryType(Character);
        }
    }
    else
    {
        // 상/하단 박스: 일반 상호작용
        UpdatePlayerEntryType(Character);
    }
}

void ACYLadderBase::OnEntryBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        // 모든 박스를 벗어났는지 체크
        if (!TopEntryBox->IsOverlappingActor(Character) &&
            !MiddleEntryBox->IsOverlappingActor(Character) &&
            !BottomEntryBox->IsOverlappingActor(Character))
        {
            PlayerEntryTypeMap.Remove(Character);
        }
        else
        {
            UpdatePlayerEntryType(Character);
        }
    }
}

void ACYLadderBase::UpdatePlayerEntryType(AActor* Player)
{
    if (!Player)
    {
        return;
    }
    
    // 우선순위: Top > Bottom > Middle
    if (TopEntryBox && TopEntryBox->IsOverlappingActor(Player))
    {
        PlayerEntryTypeMap.Add(Player, ELadderEntryType::Top);
    }
    else if (BottomEntryBox && BottomEntryBox->IsOverlappingActor(Player))
    {
        PlayerEntryTypeMap.Add(Player, ELadderEntryType::Bottom);
    }
    else if (MiddleEntryBox && MiddleEntryBox->IsOverlappingActor(Player))
    {
        PlayerEntryTypeMap.Add(Player, ELadderEntryType::Middle);
    }
}

FCYInteractionInfo ACYLadderBase::GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const
{
    const ELadderEntryType EntryType = GetPlayerEntryType(InteractionQuery.RequestingAvatar.Get());
    
    // 중간 박스 + 자동 그랩 = 상호작용 정보 없음
    if (EntryType == ELadderEntryType::Middle && bEnableAutoGrab)
    {
        return FCYInteractionInfo();
    }
    
    switch (EntryType)
    {
        case ELadderEntryType::Top:
            return ClimbDownInteractionInfo;
        case ELadderEntryType::Bottom:
            return ClimbUpInteractionInfo;
        case ELadderEntryType::Middle:
            return ClimbMiddleInteractionInfo;
        default:
            return FCYInteractionInfo();
    }
}

void ACYLadderBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
    if (LadderMesh && LadderMesh->GetStaticMesh())
    {
        OutMeshComponents.Add(LadderMesh);
    }
}

bool ACYLadderBase::CanInteraction(const FCYInteractionQuery& InteractionQuery) const
{
    if (!Super::CanInteraction(InteractionQuery))
    {
        return false;
    }
    
    const ELadderEntryType EntryType = GetPlayerEntryType(InteractionQuery.RequestingAvatar.Get());
    
    // 중간 박스는 자동 그랩 활성화시 상호작용 불가
    if (EntryType == ELadderEntryType::Middle && bEnableAutoGrab)
    {
        return false;
    }
    
    return EntryType != ELadderEntryType::None;
}

void ACYLadderBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    SetupEntryBoxes();
    
    if (!bShowDebugVisualization || !GetWorld())
    {
        return;
    }
    
    const FVector Bottom = GetBottomWorldLocation();
    const FVector Top = GetTopWorldLocation();
    const float Height = GetTotalHeight();
    const float EdgeHeight = Height * EdgeBoxHeightRatio;
    
    // 사다리 레일
    DrawDebugLine(GetWorld(), Bottom, Top, FColor::Cyan, false, -1, 0, 2.0f);
    
    // 상단 박스 (빨강)
    DrawDebugBox(GetWorld(), 
                Top - FVector(0, 0, EdgeHeight * 0.5f),
                FVector(EntryBoxRadius, EntryBoxRadius, EdgeHeight * 0.5f),
                FColor::Red, false, -1, 0, 2.0f);
    
    // 하단 박스 (초록)
    DrawDebugBox(GetWorld(),
                Bottom + FVector(0, 0, EdgeHeight * 0.5f),
                FVector(EntryBoxRadius, EntryBoxRadius, EdgeHeight * 0.5f),
                FColor::Green, false, -1, 0, 2.0f);
    
    // 중간 박스 (자동 그랩시 주황, 아니면 노랑)
    const float MiddleHeight = Height - (EdgeHeight * 2);
    if (MiddleHeight > 0)
    {
        FColor MiddleColor = bEnableAutoGrab ? FColor::Orange : FColor::Yellow;
        DrawDebugBox(GetWorld(),
                    (Bottom + Top) * 0.5f,
                    FVector(EntryBoxRadius, EntryBoxRadius, MiddleHeight * 0.5f),
                    MiddleColor, false, -1, 0, 2.0f);
    }
}






