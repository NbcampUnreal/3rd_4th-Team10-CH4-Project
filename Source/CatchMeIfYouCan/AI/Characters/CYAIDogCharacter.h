// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "CYAIDogCharacter.generated.h"

class ACYGuardDogPoolManager;

UCLASS()
class CATCHMEIFYOUCAN_API ACYAIDogCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//생성자
	ACYAIDogCharacter();

	// 레벨에 배치된 경로 액터를 지정하기 위한 변수
	UPROPERTY(EditInstanceOnly, Replicated,BlueprintReadOnly, Category = "AI Patrol")
	AActor* TargetPatrolPath;

	
	//감지 상태 변경용 함수
	void SetBarkingState(bool bNewBarking);


	
	//서버 상태 설정
	UFUNCTION(Server, Reliable)
	void ServerSetBarkingState(bool bNewBarking);

	
	// 현재 감지 상태 확인용 함수
	UFUNCTION(BlueprintCallable, Category="AI State")
	bool IsBarking() const { return bIsBarking; }

	//애니메이션용 변수 스플라인 기반이기 때문에 실제 속도랑 캐릭터 속도랑 다름
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI Animation") 
	float AISpeed;
    
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI Animation") 
	float AIDirection;	

	//이동 상태 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "AI Animation")
	void UpdateAIAnimationVariables(float NewSpeed, float NewDirection);

	
	//오브젝트 풀링 관련 함수
	void ActivateDog(FVector SpawnLocation, AActor* NewPatrolPath);
	void DeactivateDog();
	void SetPoolManager(ACYGuardDogPoolManager* InManager);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// 감시 상태 변수(상태 변경시 ONRep_IsBarking 자동으로 호출)
	UPROPERTY(ReplicatedUsing=ONRep_IsBarking, BlueprintReadOnly, Category= "AI State")
	bool bIsBarking;

	
	// 복제 콜백 함수 bIsBarking 상태 변경시 자동 호출될 함수
	UFUNCTION()
	void ONRep_IsBarking();

	//블루프린트 구현 이벤트(시각적 효과 구현전용)
	UFUNCTION(BlueprintImplementableEvent, Category= "AI Visuals")
	void OnStartBarkingVisuals();

	UFUNCTION(BlueprintImplementableEvent, Category= "AI Visuals")
	void OnStopBarkingVisuals();

private:
	UPROPERTY()
	ACYGuardDogPoolManager* PoolManager;

	// 서버에서 실제로 비활성화를 처리하는 함수
	void DeactivateDog_Internal();

	// 클라이언트에서 활성화/비활성화 시 시각 효과를 처리하기 위한 멀티캐스트 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnStateChanged(bool bIsActive);
};
