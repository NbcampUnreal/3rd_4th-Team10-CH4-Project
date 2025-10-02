#include "CYItemSpawnManager.h"
#include "Actors/CYItemSpawner.h"
#include "Items/CYItemSpawnData.h"
#include "GameModes/InGame/CYInGameState.h"
#include "EngineUtils.h"

ACYItemSpawnManager* ACYItemSpawnManager::Instance = nullptr;

ACYItemSpawnManager::ACYItemSpawnManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
	bReplicates = false;
}

ACYItemSpawnManager* ACYItemSpawnManager::GetInstance(UWorld* World)
{
	if ((!Instance || !IsValid(Instance)) && World)
	{
		Instance = nullptr;
        
		for (TActorIterator<ACYItemSpawnManager> It(World); It; ++It)
		{
			Instance = *It;
			return Instance;
		}
        
		Instance = World->SpawnActor<ACYItemSpawnManager>();
	}
	return Instance;
}

void ACYItemSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	Instance = this;

	if (SharedSpawnData && SharedSpawnData->GetAllThresholds().Num() > 0)
	{
		TArray<int32> Thresholds = SharedSpawnData->GetAllThresholds();
		Thresholds.Sort([](int32 A, int32 B) { return A > B; });
		CurrentThreshold = Thresholds[0];
	}
}

void ACYItemSpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Instance == this)
	{
		Instance = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ACYItemSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	if (!HasAuthority()) return;
    
	CheckThresholdChange();
}

void ACYItemSpawnManager::CheckThresholdChange()
{
	if (!SharedSpawnData) return;
    
	ACYInGameState* GameState = GetWorld()->GetGameState<ACYInGameState>();
	if (!GameState || GameState->GetCurrentGamePhase() != EGamePhase::InProgress)
	{
		return;
	}
    
	float RemainingTime = GameState->GetMatchRemainingTimeLocal();
    
	TArray<int32> Thresholds = SharedSpawnData->GetAllThresholds();
	Thresholds.Sort([](int32 A, int32 B) { return A > B; });
    
	// 남은 시간 이하인 Threshold 중 가장 작은 값 선택
	int32 NewThreshold = -1;
	
	for (int32 i = Thresholds.Num() - 1; i >= 0; i--)
	{
		if (RemainingTime <= Thresholds[i])
		{
			NewThreshold = Thresholds[i];
			break;
		}
	}
    
	// 모든 Threshold보다 시간이 많으면 가장 큰 Threshold 사용
	if (NewThreshold == -1 && Thresholds.Num() > 0)
	{
		NewThreshold = Thresholds[0];
	}
    
	if (NewThreshold != -1 && NewThreshold != CurrentThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Threshold changed: %d -> %d (Time: %.1f)"), 
			   CurrentThreshold, NewThreshold, RemainingTime);
		CurrentThreshold = NewThreshold;
		OnThresholdChanged.Broadcast(CurrentThreshold);
	}
}

void ACYItemSpawnManager::RegisterSpawner(ACYItemSpawner* Spawner)
{
	if (Spawner && !RegisteredSpawners.Contains(Spawner))
	{
		RegisteredSpawners.Add(Spawner);
	}
}

void ACYItemSpawnManager::UnregisterSpawner(ACYItemSpawner* Spawner)
{
	RegisteredSpawners.Remove(Spawner);
}