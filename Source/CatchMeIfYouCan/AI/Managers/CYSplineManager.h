// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CYSplineManager.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYSplineManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACYSplineManager();

	//스플라인 등록
	UFUNCTION(BlueprintCallable, Category = "Spline Management")
	void RegisterSplines(const TArray<AActor*>& Splines);
	
	//사용 가능한 스플라인 하나를 할당하고 반환
	UFUNCTION(BlueprintCallable, Category = "Spline Management")
	AActor* AssignAvailableSpline();
	
	//사용 완료된 스플라인 해제
	UFUNCTION(BlueprintCallable, Category = "Spline Management")
	void ReleaseSpline(AActor* SplineActor);
	
	//모든 스플라인 해제
	UFUNCTION(BlueprintCallable, Category = "Spline Management")
	void ReleaseAllSplines();
	
	//사용 가능한 스플라인 개수 반환
	UFUNCTION(BlueprintPure, Category = "Spline Management")
	int32 GetAvailableSplineCount() const;
	
	//전체 등록된 스플라인 개수 반환
	UFUNCTION(BlueprintPure, Category = "Spline Management")
	int32 GetTotalSplineCount() const { return AvailableSplines.Num(); }
	
	//현재 사용중인 스플라인 개수 반환
	UFUNCTION(BlueprintPure, Category = "Spline Management")
	int32 GetOccupiedSplineCount() const { return OccupiedSplines.Num(); }
	
	
	//전역에서 SplineManager 인스턴스 가져오기
	UFUNCTION(BlueprintCallable, Category = "Spline Management")
	static ACYSplineManager* GetInstance(UWorld* World);
	
	//레벨에서 GuardPatrol 태그가 있는 스플라인들을 자동으로 찾아서 등록
	UFUNCTION(BlueprintCallable, Category = "Spline Management")
	void AutoRegisterSplinesFromLevel(const FName& SplineTag = FName("GuardPatrol"));

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ========== 스플라인 데이터 ==========
	
	// 사용 가능한 스플라인들
	UPROPERTY(VisibleAnywhere, Category = "Spline Management")
	TArray<AActor*> AvailableSplines;
	
	// 현재 사용중인 스플라인들
	UPROPERTY(VisibleAnywhere, Category = "Spline Management")
	TArray<AActor*> OccupiedSplines;
	
	//싱글톤 인스턴스
	static ACYSplineManager* Instance;
	
	// ========== 내부 함수들 ==========
	
	// 스플라인이 유효한지 검사 (SplineComponent가 있는지 확인)
	bool IsValidSpline(AActor* SplineActor) const;
	
	// 중복 등록 방지
	bool IsAlreadyRegistered(AActor* SplineActor) const;
	
	// 유효하지 않은 스플라인들 정리
	void CleanupInvalidSplines();
};
