#include "CYItemSpawner.h"

#include "Components/SphereComponent.h"
#include "Items/CYItemSpawnData.h"
#include "Items/CYItemBase.h"
#include "GameModes/InGame/CYInGameState.h"
#include "Kismet/GameplayStatics.h"

ACYItemSpawner::ACYItemSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

#if WITH_EDITORONLY_DATA
	// 에디터 전용 디버그 구체
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
    
    if (!HasAuthority() || !SpawnData) return;
    
    TrySpawnItem();
}

void ACYItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void ACYItemSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
    
	// SpawnData가 변경되면 디버그 비주얼 업데이트
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ACYItemSpawner, SpawnData))
	{
		UpdateDebugVisuals();
	}
}

void ACYItemSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateDebugVisuals();
}

void ACYItemSpawner::UpdateDebugVisuals()
{
#if WITH_EDITORONLY_DATA
	if (DebugSphere && SpawnData)
	{
		// ItemCheckRadius에 맞춰 구체 크기 조정
		DebugSphere->SetSphereRadius(SpawnData->ItemCheckRadius);
        
		// 색상 설정 (초록색)
		DebugSphere->ShapeColor = FColor::Green;
        
		// 강제 업데이트
		DebugSphere->MarkRenderStateDirty();
		DebugSphere->UpdateBounds();
		DebugSphere->SetVisibility(true);
	}
#endif
}
#endif

void ACYItemSpawner::TrySpawnItem()
{
	if (!SpawnData || !HasAuthority()) return;
    
	if (HasItemInRadius())
	{
		ScheduleNextSpawn(SpawnData->NormalSpawnCooldown);
		return;
	}
    
	ACYInGameState* GameState = GetWorld()->GetGameState<ACYInGameState>();
	float RemainingTime = GameState ? GameState->GetMatchRemainingTimeLocal() : 300.0f;
    
	UE_LOG(LogTemp, Warning, TEXT("Spawner trying to spawn - Remaining Time: %.1f"), RemainingTime);
    
	TSubclassOf<ACYItemBase> SelectedItem = SpawnData->SelectRandomItem();
	if (!SelectedItem)
	{
		ScheduleNextSpawn(SpawnData->NormalSpawnCooldown);
		return;
	}
    
	SpawnItemAtLocation(SelectedItem, RemainingTime);
	ScheduleNextSpawn(SpawnData->NormalSpawnCooldown);
}

bool ACYItemSpawner::HasItemInRadius() const
{
	if (!SpawnData || !GetWorld()) return false;
    
	TArray<AActor*> FoundItems;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACYItemBase::StaticClass(), FoundItems);
    
	FVector SpawnerLoc = GetActorLocation();
	float RadiusSq = FMath::Square(SpawnData->ItemCheckRadius);
    
	for (AActor* Actor : FoundItems)
	{
		if (!Actor) continue;
        
		if (ACYItemBase* Item = Cast<ACYItemBase>(Actor))
		{
			if (!Item->bIsPickedUp && 
				FVector::DistSquared(Item->GetActorLocation(), SpawnerLoc) <= RadiusSq)
			{
				return true;
			}
		}
	}
	return false;
}

void ACYItemSpawner::SpawnItemAtLocation(TSubclassOf<ACYItemBase> ItemClass, float RemainingTime)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
	ACYItemBase* SpawnedItem = GetWorld()->SpawnActor<ACYItemBase>(
		ItemClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
    
	if (SpawnedItem)
	{
		ApplyValuesToItem(SpawnedItem, ItemClass, RemainingTime);
		CurrentSpawnedItem = SpawnedItem;
        
		SpawnedItem->OnItemPickedUpDelegate.AddDynamic(this, &ACYItemSpawner::OnSpawnedItemPickedUp);
        
		UE_LOG(LogTemp, Warning, TEXT("Spawned %s (Primary:%.1f, Duration:%.1f)"), 
			   *SpawnedItem->ItemName.ToString(), 
			   SpawnedItem->OverridePrimaryValue, 
			   SpawnedItem->OverrideDuration);
	}
}

void ACYItemSpawner::ApplyValuesToItem(ACYItemBase* Item, TSubclassOf<ACYItemBase> ItemClass, float RemainingTime)
{
	if (!Item || !SpawnData) return;
    
	FItemValueOverride Values = SpawnData->GetValuesForItem(ItemClass, RemainingTime);
    
	if (Values.PrimaryValue > 0.0f)
		Item->OverridePrimaryValue = Values.PrimaryValue;
    
	if (Values.Duration > 0.0f)
		Item->OverrideDuration = Values.Duration;
}

void ACYItemSpawner::OnSpawnedItemPickedUp(ACYItemBase* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnSpawnedItemPickedUp: Item is null"));
		return;
	}
    
	if (CurrentSpawnedItem.Get() == Item)
	{
		CurrentSpawnedItem.Reset();
        
		if (SpawnData)
		{
			ScheduleNextSpawn(SpawnData->PostPickupSpawnDelay);
		}
	}
}

void ACYItemSpawner::ScheduleNextSpawn(float Delay)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("ScheduleNextSpawn: World is null"));
		return;
	}
    
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle, this, &ACYItemSpawner::TrySpawnItem, Delay, false);
}