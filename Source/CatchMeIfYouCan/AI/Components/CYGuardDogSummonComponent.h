// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CYGuardDogSummonComponent.generated.h"

class ACYSplineManager;
class ACYGuardDogPoolManager;
class ACYAIDogCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CATCHMEIFYOUCAN_API UCYGuardDogSummonComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYGuardDogSummonComponent();

	// 사용할 수 있는 스플라인 액터 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon")
	TArray<AActor*> AvailableSplines;

	// 한 번에 소환할 경비견 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxSummonCount = 3;

	// 소환 위치 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon")
	float SpawnRadius = 200.0f;

	// 소환된 경비견 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon")
	float SummonDuration = 30.0f;

	// ✅ 쿨타임 관련 변수들 (UI 연동용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard Dog Summon")
	float SummonCooldown = 60.0f; // 총 쿨타임 (초)

	UPROPERTY(BlueprintReadOnly, Category = "Guard Dog Summon")
	float RemainingCooldown = 0.0f; // 남은 쿨타임 (초)

	// ✅ 블루프린트에서 접근 가능: 쿨타임 진행 여부
	UFUNCTION(BlueprintPure, Category = "Guard Dog Summon")
	bool IsCooldownActive() const { return RemainingCooldown > 0.0f; }

	// ✅ 쿨타임 타이머 시작 (C++ 내부 또는 블루프린트에서 수동 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Guard Dog Summon")
	void StartSummonCooldown();

	// 경비견들을 소환하는 메인 함수
	UFUNCTION(BlueprintCallable, Category = "Guard Dog Summon")
	void SummonGuardDogs(FVector SpawnLocation);

	// 모든 소환된 경비견들을 해제
	UFUNCTION(BlueprintCallable, Category = "Guard Dog Summon")
	void DismissAllDogs();

	// 서버 RPC들
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSummonGuardDogs(FVector SpawnLocation);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDismissAllDogs();

	// 상태 확인 함수들
	UFUNCTION(BlueprintPure, Category = "Guard Dog Summon")
	int32 GetActiveDogCount() const { return ActiveDogs.Num(); }

	UFUNCTION(BlueprintPure, Category = "Guard Dog Summon")
	bool CanSummon() const;

	UFUNCTION(BlueprintPure, Category = "Guard Dog Summon")
	int32 GetAvailableSplineCount() const;

	// 어빌리티가 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "GuardDogSummon")
	void AddDetectedRobber(AActor* Robber);

	UFUNCTION(BlueprintCallable, Category = "GuardDogSummon")
	void RemoveDetectedRobber(AActor* Robber);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 서버에서 실제로 실행되는 소환 로직
	void ServerExecuteSummon(FVector SpawnLocation);

private:
	// 감지된 도둑 목록
	UPROPERTY()
	TSet<AActor*> DetectedRobbers;

	// 현재 활성화된 경비견들
	UPROPERTY()
	TArray<ACYAIDogCharacter*> ActiveDogs;

	// 매니저 참조
	UPROPERTY()
	ACYGuardDogPoolManager* DogPoolManager;

	UPROPERTY()
	ACYSplineManager* SplineManager;

	// 타이머
	FTimerHandle DismissTimerHandle;
	FTimerHandle StaggeredSummonTimerHandle;
	FTimerHandle CooldownTimerHandle; // ✅ 쿨타임용 타이머 추가

	// 대기 중인 정보들
	UPROPERTY()
	TArray<FVector> PendingSpawnLocations;

	UPROPERTY()
	TArray<AActor*> PendingSplines;

	// 내부 함수들
	void FindManagers();
	TArray<FVector> CalculateSpawnPositions(FVector CenterLocation, int32 DogCount);
	FVector GetValidSpawnLocation(const FVector& DesiredLocation);
	void AutoDismissDogs();
	void CleanupInvalidDogs();
	void ActivateOneDog_Staggered();

	// ✅ 쿨타임 처리용 내부 함수
	void UpdateCooldown(); // 타이머로 남은 시간 감소
};
