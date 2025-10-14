// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Components/CYGuardDogSummonComponent.h"

#include "NavigationSystem.h"
#include "AI/Managers/CYSplineManager.h"
#include "AI/Characters/CYAIDogCharacter.h"
#include "AI/Managers/CYGuardDogPoolManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Character/CYPlayerCharacter.h"


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
	if (GetOwner() && GetOwner()->HasAuthority())
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
	Super::EndPlay(EndPlayReason);
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
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("서버 인지 확인 및 컴포넌트 소유 액터 존재 확인"));
		return;
	}
	if (!CanSummon() || ActiveDogs.Num() > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("소환된 개 있는지 확인"));
		return;
	}

	// 쿨타임 중이면 리턴
	if (RemainingCooldown > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("현재 쿨타임 진행 중입니다. (남은 시간: %.1f초)"), RemainingCooldown);
		return;
	}

	// 소환 중인지 확인
	if (GetWorld()->GetTimerManager().IsTimerActive(StaggeredSummonTimerHandle))
	{
		return;
	}

	//사용중인 경비견 제거
	CleanupInvalidDogs();

	//활성화된 스플라인과 소환 가능한 경비견 중 더 작은 수를 할당
	int32 DogsToSummon = FMath::Min(MaxSummonCount, GetAvailableSplineCount());
	if (DogsToSummon <= 0) return;
	GetWorld()->GetTimerManager().ClearTimer(StaggeredSummonTimerHandle);
	PendingSpawnLocations.Empty();
	PendingSplines.Empty();

	// ✅ NavMesh 유효 위치로 스폰 좌표 보정 추가
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLoc;
		if (NavSys->ProjectPointToNavigation(SpawnLocation, ProjectedLoc, FVector(200, 200, 500)))
		{
			SpawnLocation = ProjectedLoc.Location;
			UE_LOG(LogTemp, Log, TEXT("NavMesh 보정 완료 -> 새로운 스폰 위치: %s"), *SpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NavMesh 보정 실패 -> 원래 위치 사용: %s"), *SpawnLocation.ToString());
		}
	}

	// 🔹 여기부터 기존 로직 그대로 유지 🔹
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

	GetWorld()->GetTimerManager().SetTimer(
		StaggeredSummonTimerHandle,
		this,
		&UCYGuardDogSummonComponent::ActivateOneDog_Staggered,
		0.3f,
		true
	);

	// ✅ 쿨타임 시작
	StartSummonCooldown();
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
		//경비견에게 오너 정보를 설정합니다.
		NewDog->SetSummoner(GetOwner());
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

FVector UCYGuardDogSummonComponent::GetValidSpawnLocation(const FVector& DesiredLocation)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return DesiredLocation;
	}

	FNavLocation ProjectedLocation;
	bool bOnNavMesh = NavSys->ProjectPointToNavigation(
		DesiredLocation,
		ProjectedLocation,
		FVector(200.0f, 200.0f, 500.0f) // 탐색 범위 (필요시 조정 가능)
	);

	if (bOnNavMesh)
	{
		return ProjectedLocation.Location; // NavMesh 위 좌표 리턴
	}

	return DesiredLocation; // 실패 시 원래 좌표 유지
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
	UE_LOG(LogTemp, Error, TEXT("===== CanSummon 체크 ====="));

	if (!DogPoolManager)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ DogPoolManager가 NULL입니다"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("✅ DogPoolManager 있음: %s"), *DogPoolManager->GetName());
	}

	if (!SplineManager)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ SplineManager가 NULL입니다"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("✅ SplineManager 있음: %s"), *SplineManager->GetName());
	}

	int32 AvailableCount = GetAvailableSplineCount();
	if (AvailableCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ 사용 가능한 스플라인이 없습니다 (개수: %d)"), AvailableCount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("✅ 사용 가능한 스플라인: %d개"), AvailableCount);
	}

	bool bCanSummon = DogPoolManager && SplineManager && AvailableCount > 0;
	UE_LOG(LogTemp, Error, TEXT("===== CanSummon 결과: %s ====="), bCanSummon ? TEXT("TRUE") : TEXT("FALSE"));
	return bCanSummon;
}

//사용되지 않은 스플라인 개수 가져오기
int32 UCYGuardDogSummonComponent::GetAvailableSplineCount() const
{
	return SplineManager ? SplineManager->GetAvailableSplineCount() : 0;
}

//추후 클라이언트의 비정상적인 요청 차단
bool UCYGuardDogSummonComponent::ServerSummonGuardDogs_Validate(FVector SpawnLocation) { return true; }
bool UCYGuardDogSummonComponent::ServerDismissAllDogs_Validate() { return true; }

void UCYGuardDogSummonComponent::AddDetectedRobber(AActor* Robber)
{
	if (!Robber || !GetOwner()->HasAuthority()) return;

	bool bWasEmpty = DetectedRobbers.Num() == 0;
	DetectedRobbers.Add(Robber);

	if (bWasEmpty)
	{
		if (ACYPlayerCharacter* OwnerCharacter = Cast<ACYPlayerCharacter>(GetOwner()))
		{
			OwnerCharacter->Client_ShowRobberDetectedWarning(true, Robber);
		}
	}
}

void UCYGuardDogSummonComponent::RemoveDetectedRobber(AActor* Robber)
{
	if (!Robber || !GetOwner()->HasAuthority()) return;

	DetectedRobbers.Remove(Robber);

	if (DetectedRobbers.Num() == 0)
	{
		if (ACYPlayerCharacter* OwnerCharacter = Cast<ACYPlayerCharacter>(GetOwner()))
		{
			OwnerCharacter->Client_ShowRobberDetectedWarning(false, Robber);
		}
	}
}


// ✅ 쿨타임 시작 함수
void UCYGuardDogSummonComponent::StartSummonCooldown()
{
	if (RemainingCooldown > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("쿨타임 중이라 StartSummonCooldown 무시됨."));
		return;
	}

	RemainingCooldown = SummonCooldown;

	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&UCYGuardDogSummonComponent::UpdateCooldown,
		1.0f,
		true
	);

	UE_LOG(LogTemp, Log, TEXT("쿨타임 시작: %.1f초"), SummonCooldown);
}

// ✅ 쿨타임 감소 함수
void UCYGuardDogSummonComponent::UpdateCooldown()
{
	if (RemainingCooldown <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
		RemainingCooldown = 0.0f;

		UE_LOG(LogTemp, Log, TEXT("쿨타임 종료."));
		return;
	}

	RemainingCooldown -= 1.0f;
	UE_LOG(LogTemp, Log, TEXT("쿨타임 남은 시간: %.1f초"), RemainingCooldown);
}
