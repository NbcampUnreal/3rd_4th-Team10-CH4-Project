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
	if (!Instance&&World)
	{
		//월드에서 PoolManager 가져오기
		for (TActorIterator<ACYGuardDogPoolManager>It(World);It;++It)
		{
			Instance = *It;
			break;
		}
	}
	return Instance;
}

//풀 초기화및 경비견 생성
void ACYGuardDogPoolManager::InitializePool()
{
	//서버가 아니거나 가져올 클래스가 없을경우 리턴
	if (!HasAuthority()|| !PooledDogClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuardDogPoolManager: PooledDogClass가 설정되지 않았거나 서버가 아닙니다."));
		return;
	}

	for (int32 i = 0 ; i<PoolSize ; i++)
	{
		FActorSpawnParameters SpawnParams;
		//풀매니저가 소유
		SpawnParams.Owner=this;
		//충돌 무시하고 무조건 생성
		
		ACYAIDogCharacter* NewDog = GetWorld()->SpawnActor<ACYAIDogCharacter>(PooledDogClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (NewDog)
		{
			//TODO 풀 매니저 참조 설정 및 생석 직후 비활성화
			AvailablePool.Add(NewDog);
			
		}
		UE_LOG(LogTemp, Log, TEXT("GuardDogPoolManager: %d개의 경비견으로 풀 초기화 완료."), PoolSize);
	}
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
