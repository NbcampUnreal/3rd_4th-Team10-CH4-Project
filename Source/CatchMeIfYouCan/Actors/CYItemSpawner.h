#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/CYItemSpawnData.h"
#include "CYTypes/CYInGameTypes.h"
#include "CYItemSpawner.generated.h"

class USphereComponent;

UCLASS()
class CATCHMEIFYOUCAN_API ACYItemSpawner : public AActor
{
	GENERATED_BODY()
    
public:
	ACYItemSpawner();

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Spawner")
	USphereComponent* DebugSphere;
#endif
    
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
	UFUNCTION()
	void OnGamePhaseChanged(EGamePhase NewPhase);
    
	UFUNCTION()
	void OnThresholdChanged(int32 NewThreshold);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void UpdateDebugVisuals();
#endif
    
private:
	void TrySpawnItem();
	void SpawnItemWithSpec(const FItemSpec& Spec);
    
	UFUNCTION()
	void OnSpawnedItemPickedUp(ACYItemBase* Item);
    
	void ScheduleNextSpawn(float Delay);
    
	FTimerHandle SpawnTimerHandle;
    
	UPROPERTY()
	TWeakObjectPtr<ACYItemBase> CurrentSpawnedItem;

	bool bFirstSpawnTriggered = false;
    
	// Spawner 자체에 현재 Threshold 저장
	int32 CurrentThreshold = 400;
};