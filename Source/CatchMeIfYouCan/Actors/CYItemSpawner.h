#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/CYItemSpawnData.h"
#include "Components/SphereComponent.h"
#include "CYItemSpawner.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYItemSpawner : public AActor
{
	GENERATED_BODY()
    
public:
	ACYItemSpawner();
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawner")
	UCYItemSpawnData* SpawnData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Spawner")
	USphereComponent* DebugSphere;
#endif
    
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnConstruction(const FTransform& Transform) override;
    
	void UpdateDebugVisuals();
#endif
    
private:
	void TrySpawnItem();
	bool HasItemInRadius() const;
	void SpawnItemAtLocation(TSubclassOf<ACYItemBase> ItemClass, float Multiplier);
	void ApplyValuesToItem(ACYItemBase* Item, TSubclassOf<ACYItemBase> ItemClass, float RemainingTime);
    
	UFUNCTION()
	void OnSpawnedItemPickedUp(ACYItemBase* Item);
    
	void ScheduleNextSpawn(float Delay);
    
	FTimerHandle SpawnTimerHandle;
    
	UPROPERTY()
	TWeakObjectPtr<ACYItemBase> CurrentSpawnedItem;
};