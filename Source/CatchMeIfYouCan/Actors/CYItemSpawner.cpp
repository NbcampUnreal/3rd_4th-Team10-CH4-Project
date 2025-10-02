#include "Actors/CYItemSpawner.h"
#include "Components/SphereComponent.h"
#include "Items/CYItemSpawnData.h"
#include "Items/CYItemBase.h"
#include "GameModes/InGame/CYInGameState.h"
#include "Items/CYItemSpawnManager.h"

ACYItemSpawner::ACYItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

#if WITH_EDITORONLY_DATA
	DebugSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DebugSphere"));
	DebugSphere->SetupAttachment(RootComponent);
	DebugSphere->SetSphereRadius(100.0f);
	DebugSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DebugSphere->SetHiddenInGame(true);
	DebugSphere->bDrawOnlyIfSelected = false;
	DebugSphere->ShapeColor = FColor::Green;
    
	if (!IsRunningCommandlet())
	{
		DebugSphere->SetVisibility(true);
	}
#endif
}

void ACYItemSpawner::BeginPlay()
{
	Super::BeginPlay();
    
	if (!HasAuthority()) return;
    
	// Manager 등록
	if (ACYItemSpawnManager* Manager = ACYItemSpawnManager::GetInstance(GetWorld()))
	{
		Manager->RegisterSpawner(this);
		Manager->OnThresholdChanged.AddDynamic(this, &ACYItemSpawner::OnThresholdChanged);
	}
    
	if (UWorld* World = GetWorld())
	{
		if (ACYInGameState* GameState = World->GetGameState<ACYInGameState>())
		{
			GameState->OnGamePhaseChanged.AddUObject(this, &ACYItemSpawner::OnGamePhaseChanged);
            
			if (GameState->GetCurrentGamePhase() == EGamePhase::InProgress)
			{
				bFirstSpawnTriggered = true;
				GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
				{
					TrySpawnItem();
				});
			}
		}
	}
}

void ACYItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
        
		if (ACYInGameState* GameState = World->GetGameState<ACYInGameState>())
		{
			GameState->OnGamePhaseChanged.RemoveAll(this);
		}
        
		if (ACYItemSpawnManager* Manager = ACYItemSpawnManager::GetInstance(World))
		{
			Manager->UnregisterSpawner(this);
		}
	}
    
	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void ACYItemSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateDebugVisuals();
}

void ACYItemSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateDebugVisuals();
}

void ACYItemSpawner::UpdateDebugVisuals()
{
#if WITH_EDITORONLY_DATA
	if (DebugSphere)
	{
		ACYItemSpawnManager* Manager = ACYItemSpawnManager::GetInstance(GetWorld());
		if (Manager && Manager->SharedSpawnData)
		{
			DebugSphere->SetSphereRadius(Manager->SharedSpawnData->ItemCheckRadius);
		}
		DebugSphere->ShapeColor = FColor::Green;
		DebugSphere->MarkRenderStateDirty();
		DebugSphere->UpdateBounds();
		DebugSphere->SetVisibility(true);
	}
#endif
}
#endif

void ACYItemSpawner::OnGamePhaseChanged(EGamePhase NewPhase)
{
	if (NewPhase == EGamePhase::InProgress && !bFirstSpawnTriggered)
	{
		bFirstSpawnTriggered = true;
		TrySpawnItem();
	}
}

void ACYItemSpawner::OnThresholdChanged(int32 NewThreshold)
{
	if (CurrentSpawnedItem.IsValid())
	{
		CurrentSpawnedItem->Destroy();
		CurrentSpawnedItem.Reset();
	}
    
	ACYItemSpawnManager* Manager = ACYItemSpawnManager::GetInstance(GetWorld());
	if (!Manager || !Manager->SharedSpawnData) return;
    
	FItemSpec SelectedSpec = Manager->SharedSpawnData->SelectRandomItemForThreshold(NewThreshold);
	if (!SelectedSpec.ItemClass) return;
    
	SpawnItemWithSpec(SelectedSpec);
}

void ACYItemSpawner::TrySpawnItem()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;
    
	if (CurrentSpawnedItem.IsValid())
	{
		ScheduleNextSpawn(30.0f);
		return;
	}
    
	ACYInGameState* GameState = World->GetGameState<ACYInGameState>();
	if (!GameState || GameState->GetCurrentGamePhase() != EGamePhase::InProgress)
	{
		ScheduleNextSpawn(30.0f);
		return;
	}
    
	ACYItemSpawnManager* Manager = ACYItemSpawnManager::GetInstance(World);
	if (!Manager || !Manager->SharedSpawnData) return;
    
	int32 CurrentThreshold = Manager->GetCurrentThreshold();
	
	UE_LOG(LogTemp, Error, TEXT("=== TrySpawnItem: Threshold=%d, RemainingTime=%.1f ==="), 
		   CurrentThreshold, GameState->GetMatchRemainingTimeLocal());
	
	FItemSpec SelectedSpec = Manager->SharedSpawnData->SelectRandomItemForThreshold(CurrentThreshold);

	UE_LOG(LogTemp, Error, TEXT("Selected Spec: Primary=%.1f, Duration=%.1f"), 
		   SelectedSpec.PrimaryValue, SelectedSpec.Duration);
	
	if (!SelectedSpec.ItemClass) return;
    
	SpawnItemWithSpec(SelectedSpec);
	ScheduleNextSpawn(Manager->SharedSpawnData->NormalSpawnCooldown);
}

void ACYItemSpawner::SpawnItemWithSpec(const FItemSpec& Spec)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
	ACYItemBase* SpawnedItem = GetWorld()->SpawnActor<ACYItemBase>(
		Spec.ItemClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
    
	if (SpawnedItem)
	{
		if (Spec.PrimaryValue > 0.0f)
			SpawnedItem->OverridePrimaryValue = Spec.PrimaryValue;
        
		if (Spec.Duration > 0.0f)
			SpawnedItem->OverrideDuration = Spec.Duration;
        
		CurrentSpawnedItem = SpawnedItem;
		SpawnedItem->OnItemPickedUpDelegate.AddDynamic(this, &ACYItemSpawner::OnSpawnedItemPickedUp);
        
		UE_LOG(LogTemp, Warning, TEXT("Spawned %s (Primary:%.1f, Duration:%.1f)"), 
			   *SpawnedItem->ItemName.ToString(), 
			   SpawnedItem->OverridePrimaryValue, 
			   SpawnedItem->OverrideDuration);
	}
}

void ACYItemSpawner::OnSpawnedItemPickedUp(ACYItemBase* Item)
{
	if (!Item) return;
    
	if (CurrentSpawnedItem.Get() == Item)
	{
		CurrentSpawnedItem.Reset();
        
		ACYItemSpawnManager* Manager = ACYItemSpawnManager::GetInstance(GetWorld());
		float Delay = Manager && Manager->SharedSpawnData ? 
			Manager->SharedSpawnData->PostPickupSpawnDelay : 10.0f;
        
		ScheduleNextSpawn(Delay);
	}
}

void ACYItemSpawner::ScheduleNextSpawn(float Delay)
{
	UWorld* World = GetWorld();
	if (!World) return;
    
	World->GetTimerManager().SetTimer(
		SpawnTimerHandle, this, &ACYItemSpawner::TrySpawnItem, Delay, false);
}