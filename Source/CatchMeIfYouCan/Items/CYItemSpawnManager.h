#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CYItemSpawnManager.generated.h"

class ACYItemSpawner;
class UCYItemSpawnData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThresholdChanged, int32, NewThreshold);

UCLASS()
class CATCHMEIFYOUCAN_API ACYItemSpawnManager : public AActor
{
	GENERATED_BODY()

public:
	ACYItemSpawnManager();
    
	static ACYItemSpawnManager* GetInstance(UWorld* World);
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	UCYItemSpawnData* SharedSpawnData;
    
	int32 GetCurrentThreshold() const { return CurrentThreshold; }
    
	void RegisterSpawner(ACYItemSpawner* Spawner);
	void UnregisterSpawner(ACYItemSpawner* Spawner);
    
	UPROPERTY(BlueprintAssignable)
	FOnThresholdChanged OnThresholdChanged;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void CheckThresholdChange();
    
	UPROPERTY()
	TArray<ACYItemSpawner*> RegisteredSpawners;
    
	int32 CurrentThreshold = -1;
	static ACYItemSpawnManager* Instance;
};