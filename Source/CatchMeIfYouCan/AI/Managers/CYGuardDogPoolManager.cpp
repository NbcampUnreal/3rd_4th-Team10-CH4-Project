// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Managers/CYGuardDogPoolManager.h"

#include "EngineUtils.h"
#include "AI/Characters/CYAIDogCharacter.h"

ACYGuardDogPoolManager* ACYGuardDogPoolManager::Instance = nullptr;

ACYGuardDogPoolManager::ACYGuardDogPoolManager()
{
	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void ACYGuardDogPoolManager::BeginPlay()
{
	Super::BeginPlay();
	Instance = this;

	//호스트만 풀을 초기화
	if (HasAuthority())
	{
		InitializePool();
	}
}

void ACYGuardDogPoolManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Instance==this)
	{
		Instance = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

//인스턴스 가져오기
ACYGuardDogPoolManager* ACYGuardDogPoolManager::GetInstance(UWorld* World)
{
	// static Instance 변수를 사용하지 않고, 항상 월드에서 액터를 찾습니다.
	if (World)
	{
		for (TActorIterator<ACYGuardDogPoolManager> It(World); It; ++It)
		{
			// 월드에 존재하는 첫 번째 인스턴스를 즉시 반환합니다.
			return *It;
		}
	}
	
	// 찾지 못했다면 nullptr을 반환합니다.
	return nullptr;
}

//풀 초기화및 경비견 생성
void ACYGuardDogPoolManager::InitializePool()
{
	UE_LOG(LogTemp, Error, TEXT("===== InitializePool 시작 ====="));
    
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Authority 없음"));
		return;
	}
    
	if (!PooledDogClass)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ PooledDogClass가 설정되지 않음!"));
		UE_LOG(LogTemp, Error, TEXT("   해결: BP_GuardDogPoolManager에서 PooledDogClass 설정"));
		return;
	}
    
	UE_LOG(LogTemp, Error, TEXT("Pool Size: %d개 생성 예정"), PoolSize);
    
	for (int32 i = 0; i < PoolSize; i++)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
		ACYAIDogCharacter* NewDog = GetWorld()->SpawnActor<ACYAIDogCharacter>(
			PooledDogClass, 
			FVector(0, 0, -10000), 
			FRotator::ZeroRotator, 
			SpawnParams
		);
        
		if (NewDog)
		{
			NewDog->SetPoolManager(this);
			NewDog->DeactivateDog();
			AvailablePool.Add(NewDog);
			UE_LOG(LogTemp, Error, TEXT("✅ 경비견 %d번 생성 완료"), i+1);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("❌ 경비견 %d번 생성 실패"), i+1);
		}
	}
    
	UE_LOG(LogTemp, Error, TEXT("===== 최종 풀 크기: %d개 ====="), AvailablePool.Num());
}

//풀에서 가져 오기 및 사용중인 경비견 배열에 추가
ACYAIDogCharacter* ACYGuardDogPoolManager::GetPooleDog()
{
	if (!HasAuthority()) return nullptr;

	//풀에 사용가능한 경비견이 있는지 확인
	if (AvailablePool.Num()>0)
	{
		ACYAIDogCharacter* Dog=AvailablePool.Pop();
		UsedPool.Add(Dog);
		return Dog;
	}
	else
	{
		//풀이 비었을때 올 로그
		UE_LOG(LogTemp, Warning, TEXT("GuardDogPoolManager: 사용 가능한 경비견이 없습니다. 풀 확장이 필요할 수 있습니다."));
		return nullptr;
	}
}

//풀에 반납 및 사용중인 경배견 배열에서 제거
void ACYGuardDogPoolManager::ReturnDogToPool(ACYAIDogCharacter* DogToReturn)
{
	if (!HasAuthority()||!DogToReturn) return;

	if (UsedPool.Remove(DogToReturn))
	{
		AvailablePool.Add(DogToReturn);
	}
}
