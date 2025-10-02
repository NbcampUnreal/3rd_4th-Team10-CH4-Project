// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CYGuardDogSummonComponent.generated.h"


class ACYSplineManager;
class ACYGuardDogPoolManager;
class ACYAIDogCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CATCHMEIFYOUCAN_API UCYGuardDogSummonComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCYGuardDogSummonComponent();

	//사용할수 있는 액터 저장할 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon")
	TArray<AActor*> AvailableSplines;

	//한 번에 소환할 경비견 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxSummonCount = 3;

	// 소환 위치에서 경비견들 간의 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon")
	float SpawnRadius = 200.0f;

	// 소환된 경비견들의 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon")
	float SummonDuration = 30.0f;

	
	// 경비견들을 소환하는 메인 함수
	UFUNCTION(BlueprintCallable, Category = "Guard Dog Summon")
	void SummonGuardDogs(FVector SpawnLocation);

	// 모든 소환된 경비견들을 해제
	UFUNCTION(BlueprintCallable, Category = "Guard Dog Summon")
	void DismissAllDogs();

	// 클라이언트가 서버에 소환 요청을 보내는 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSummonGuardDogs(FVector SpawnLocation);

	// 클라이언트가 서버에 해제 요청을 보내는 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDismissAllDogs();

	// 현재 활성화된 경비견 수 반환
	UFUNCTION(BlueprintPure, Category = "Guard Dog Summon")
	int32 GetActiveDogCount() const { return ActiveDogs.Num(); }

	// 소환 가능한지 체크
	UFUNCTION(BlueprintPure, Category = "Guard Dog Summon")
	bool CanSummon() const;

	// 사용 가능한 스플라인 수 반환
	UFUNCTION(BlueprintPure, Category = "Guard Dog Summon")
	int32 GetAvailableSplineCount() const;

	// 어빌리티가 호출할 함수들입니다.
	UFUNCTION(BlueprintCallable, Category = "GuardDogSummon")
	void AddDetectedRobber(AActor* Robber);
	
	UFUNCTION(BlueprintCallable, Category = "GuardDogSummon")
	void RemoveDetectedRobber(AActor* Robber);
	
protected:
	virtual void BeginPlay() override;

	// 컴포넌트가 제거될 때 호출
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 서버에서 실제로 실행되는 소환 로직
	void ServerExecuteSummon(FVector SpawnLocation);
	
private:

	// ========== 내부 변수들 ==========
	//감지된 모든 도둑의 목록 저장
	UPROPERTY()
	TSet<AActor*> DetectedRobbers;
	
	// 현재 풀에서 활성화된 경비견들
	UPROPERTY()
	TArray<ACYAIDogCharacter*> ActiveDogs;

	// 오브젝트 풀 매니저 참조
	UPROPERTY()
	ACYGuardDogPoolManager* DogPoolManager;

	// 스플라인 매니저 참조
	UPROPERTY()
	ACYSplineManager* SplineManager;

	// 자동 해제용 타이머
	FTimerHandle DismissTimerHandle;
    
	// 단계적 활성화를 위한 타이머
	FTimerHandle StaggeredSummonTimerHandle;

	// 활성화를 대기 중인 위치 정보 배열
	UPROPERTY()
	TArray<FVector> PendingSpawnLocations;
    
	// 할당 대기 중인 스플라인 정보 배열
	UPROPERTY()
	TArray<AActor*> PendingSplines;

	// ========== 내부 함수들 ==========

	// 레벨에 있는 매니저들(스플라인, 오브젝트 풀) 찾기
	void FindManagers();
    
	// 소환 위치들 계산 (원형 배치)
	TArray<FVector> CalculateSpawnPositions(FVector CenterLocation, int32 DogCount);
    
	// 타이머로 자동 해제
	void AutoDismissDogs();
    
	// 유효하지 않은(죽거나 제거된) 경비견 참조 정리
	void CleanupInvalidDogs();
    
	// 타이머가 호출할 실제 활성화 함수
	void ActivateOneDog_Staggered();
};
