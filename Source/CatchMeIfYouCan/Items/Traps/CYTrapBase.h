#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "CYTrapBase.generated.h"

class UGameplayEffect;
class ACYPlayerCharacter;

UENUM(BlueprintType)
enum class ETrapType : uint8
{
    Slow        UMETA(DisplayName = "Slow Trap"),
    Freeze      UMETA(DisplayName = "Freeze Trap"),
    Damage      UMETA(DisplayName = "Damage Trap")
};

UENUM(BlueprintType)
enum class ETrapState : uint8
{
    MapPlaced       UMETA(DisplayName = "Map Placed (Pickupable)"),
    PlayerPlaced    UMETA(DisplayName = "Player Placed (Active)")
};

UCLASS(Abstract)
class CATCHMEIFYOUCAN_API ACYTrapBase : public ACYItemBase
{
    GENERATED_BODY()

public:
    ACYTrapBase();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    ETrapType TrapType = ETrapType::Slow;

    UPROPERTY(ReplicatedUsing = OnRep_TrapState, BlueprintReadOnly, Category = "Trap")
    ETrapState TrapState = ETrapState::MapPlaced;

    UPROPERTY(ReplicatedUsing = OnRep_IsArmed, BlueprintReadOnly, Category = "Trap")
    bool bIsArmed = false;

	UPROPERTY(Replicated)
	bool bHasTriggered = false;

    // 트랩 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    float TriggerRadius = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    float ArmingDelay = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    float TrapLifetime = 60.0f;

    // 트랩 효과들
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    TArray<TSubclassOf<UGameplayEffect>> TrapEffects;

    // 트랩 전용 함수들
    virtual bool UseItem(ACYPlayerCharacter* Character) override;

    UFUNCTION(BlueprintCallable, Category = "Trap")
    void PlaceTrap(const FVector& Location, ACYPlayerCharacter* Placer);

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 네트워크 동기화
    UFUNCTION()
    void OnRep_TrapState();

    UFUNCTION()
    void OnRep_IsArmed();

    // 트랩 로직
    UFUNCTION()
    void ArmTrap();

    UFUNCTION()
    virtual void OnTrapTriggered(ACYPlayerCharacter* Target);

    // 충돌 이벤트 (오버라이드)
    UFUNCTION()
    void OnTrapSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // 트랩별 커스텀 효과 (하위 클래스에서 구현)
    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void ApplyTrapEffect(ACYPlayerCharacter* Target);

private:
    FTimerHandle ArmingTimer;
    FTimerHandle LifetimeTimer;
};