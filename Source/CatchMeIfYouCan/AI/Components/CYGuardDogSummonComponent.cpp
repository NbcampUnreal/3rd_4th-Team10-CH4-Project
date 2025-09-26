// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Components/CYGuardDogSummonComponent.h"
#include "AI/Managers/CYSplineManager.h"
#include "AI/Characters/CYAIDogCharacter.h"
#include "AI/Managers/CYGuardDogPoolManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UCYGuardDogSummonComponent::UCYGuardDogSummonComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
}
void UCYGuardDogSummonComponent::FindManagers()
{
	UWorld* World = GetWorld();
	if (!World) return;

	SplineManager = ACYSplineManager::GetInstance(World);
	DogPoolManager = ACYGuardDogPoolManager::GetInstance(World);

	if (!SplineManager) UE_LOG(LogTemp, Warning, TEXT("GuardDogSummonComponent: 스플라인 매니저를 찾을 수 없습니다."));
	if (!DogPoolManager) UE_LOG(LogTemp, Warning, TEXT("GuardDogSummonComponent: 경비견 풀 매니저를 찾을 수 없습니다."));
}

void UCYGuardDogSummonComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner()&& GetOwner()->HasAuthority())
	{
		//스플라인 매니저 및 풀 매니저 찾아서 연결
		FindManagers();
		
		//컴포넌트에서 미리 할당한 스플라인 추가
		if (SplineManager && AvailableSplines.Num() > 0)
		{
			SplineManager->RegisterSplines(AvailableSplines);
		}
	}
	
}

void UCYGuardDogSummonComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//서버에서만 관리
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DismissAllDogs();
	}
}

void UCYGuardDogSummonComponent::SummonGuardDogs(FVector SpawnLocation)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerSummonGuardDogs(SpawnLocation);
		return;
	}
	//서버에서는 로컬 함수 호출
	ServerExecuteSummon(SpawnLocation);
}



//서버에 소환 요청
void UCYGuardDogSummonComponent::ServerSummonGuardDogs_Implementation(FVector SpawnLocation)
{
	ServerExecuteSummon(SpawnLocation);
}

void UCYGuardDogSummonComponent::ServerExecuteSummon(FVector SpawnLocation)
{
	//서버가 아니면 리턴 및 소환 가능한 상태인지 확인
	if (!GetOwner()||!GetOwner()->HasAuthority()) return;
	if (!CanSummon() || ActiveDogs.Num() > 0) return;

	// 소환 중인지 확인
	if (GetWorld()->GetTimerManager().IsTimerActive(StaggeredSummonTimerHandle))
	{
		return;
	}
	
	//사용중인 경비견 제거
	CleanupInvalidDogs();

	//활성화된 스플라인과 소환 가능한 경비견 중 더 작은 수를 할당
	int32 DogsToSummon = FMath::Min(MaxSummonCount, GetAvailableSplineCount());
	if (DogsToSummon <=0)return;
	GetWorld()->GetTimerManager().ClearTimer(StaggeredSummonTimerHandle);
	PendingSpawnLocations.Empty();
	PendingSplines.Empty();

	PendingSpawnLocations = CalculateSpawnPositions(SpawnLocation, DogsToSummon);
	for (int32 i = 0; i < DogsToSummon; ++i)
	{
		//스플라인 매니저에서 사용중인 스플라인 설정
		if (AActor* AssignedSpline = SplineManager ? SplineManager->AssignAvailableSpline() : nullptr)
		{
			PendingSplines.Add(AssignedSpline);
		}
		//스플라인이 없다면 위치 정보 제거
		else
		{
			PendingSpawnLocations.RemoveAt(i);
			break;
		}
	}

	GetWorld()->GetTimerManager().SetTimer(StaggeredSummonTimerHandle, this, &UCYGuardDogSummonComponent::ActivateOneDog_Staggered, 0.3f, true);
	
}


void UCYGuardDogSummonComponent::ActivateOneDog_Staggered()
{
    if (PendingSpawnLocations.Num() == 0 || PendingSplines.Num() == 0 || !DogPoolManager)
    {
    	GetWorld()->GetTimerManager().ClearTimer(StaggeredSummonTimerHandle);
    	if (SummonDuration > 0.0f && ActiveDogs.Num() > 0)
    	{
    		GetWorld()->GetTimerManager().SetTimer(DismissTimerHandle, this, &UCYGuardDogSummonComponent::AutoDismissDogs, SummonDuration, false);
    	}
    	return;
    }

	
	FVector SpawnPos = PendingSpawnLocations[0];
	PendingSpawnLocations.RemoveAt(0);

	AActor* Spline = PendingSplines[0];
	PendingSplines.RemoveAt(0);

	ACYAIDogCharacter* NewDog = DogPoolManager->GetPooleDog();
	if (NewDog)
	{
		//경비견 활성화
		NewDog->ActivateDog(SpawnPos, Spline);
		//활성화 배열에 추가
		ActiveDogs.Add(NewDog);
	}
	else
	{
		//만약 실패시 스플라인 반환
		if (SplineManager) SplineManager->ReleaseSpline(Spline);
	}
}

void UCYGuardDogSummonComponent::DismissAllDogs()
{
	//서버가 아닐경우
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerDismissAllDogs();
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(DismissTimerHandle);

	for (ACYAIDogCharacter* Dog : ActiveDogs)
	{
		if (Dog && IsValid(Dog))
		{
			Dog->DeactivateDog();
		}
	}
	ActiveDogs.Empty();
}

//원형 소환 함수
TArray<FVector> UCYGuardDogSummonComponent::CalculateSpawnPositions(FVector CenterLocation, int32 DogCount)
{
	TArray<FVector> Positions;
	if (DogCount <= 0) return Positions;
	if (DogCount == 1)
	{
		Positions.Add(CenterLocation);
		return Positions;
	}
	for (int32 i = 0; i < DogCount; i++)
	{
		float Angle = (360.0f / DogCount) * i;
		float RadianAngle = FMath::DegreesToRadians(Angle);
		FVector Offset = FVector(FMath::Cos(RadianAngle) * SpawnRadius, FMath::Sin(RadianAngle) * SpawnRadius, 0.0f);
		Positions.Add(CenterLocation + Offset);
	}
	return Positions;
}

void UCYGuardDogSummonComponent::AutoDismissDogs()
{
	DismissAllDogs();
}

void UCYGuardDogSummonComponent::CleanupInvalidDogs()
{
	ActiveDogs.RemoveAll([](const ACYAIDogCharacter* Dog) { return !Dog || !IsValid(Dog); });
}



void UCYGuardDogSummonComponent::ServerDismissAllDogs_Implementation()
{
	DismissAllDogs();
}
//소환 가능한지 확인
bool UCYGuardDogSummonComponent::CanSummon() const
{
	return DogPoolManager && SplineManager && GetAvailableSplineCount() > 0;
}

//사용되지 않은 스플라인 개수 가져오기
int32 UCYGuardDogSummonComponent::GetAvailableSplineCount() const
{
	return SplineManager ? SplineManager->GetAvailableSplineCount() : 0;
}

//추후 클라이언트의 비정상적인 요청 차단
bool UCYGuardDogSummonComponent::ServerSummonGuardDogs_Validate(FVector SpawnLocation){return true;}
bool UCYGuardDogSummonComponent::ServerDismissAllDogs_Validate() { return true; }
