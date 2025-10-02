// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CYGuardDogPoolManager.generated.h"

class ACYAIDogCharacter;

UCLASS()
class CATCHMEIFYOUCAN_API ACYGuardDogPoolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ACYGuardDogPoolManager();

	//인스턴스 가져오기
	UFUNCTION(BlueprintCallable, Category ="Object Pool")
	static ACYGuardDogPoolManager* GetInstance(UWorld* World);

	//풀에서 사용 가능한 경비견 가져오기
	ACYAIDogCharacter* GetPooleDog();

	//사용이 끝난 경비견을 다시 풀로 가져오기
	void ReturnDogToPool(ACYAIDogCharacter* DogReturn);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	//가져올 경비견 클래스 지정
	UPROPERTY(EditAnywhere, Category="Object Pool")
	TSubclassOf<ACYAIDogCharacter> PooledDogClass;
	
	//생성할 경비견의 수
	UPROPERTY(EditAnywhere, Category="Object Pool",meta=(ClampMin="1",ClampMax="10"))
	int32 PoolSize=10;

	//사용 가능한 경비견을 담는 배열
	UPROPERTY()
	TArray<ACYAIDogCharacter*> AvailablePool;

	//사용중인 경비견을 담을 배열
	UPROPERTY()
	TArray<ACYAIDogCharacter*> UsedPool;

	//싱글톤
	static ACYGuardDogPoolManager* Instance;

	//풀 초기화및 경비견 생성
	void InitializePool();
	
};
