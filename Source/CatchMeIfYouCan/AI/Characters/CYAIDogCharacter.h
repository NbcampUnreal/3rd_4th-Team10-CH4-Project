// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "CYAIDogCharacter.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYAIDogCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//생성자
	ACYAIDogCharacter();

	//이동할 동선 지정용
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI Patrol")
	AActor* TargetPatrolPath;


	
	//감지 상태 설정용 함수
	void SetBarkingState(bool bNewBarking);


	
	// 서버 RPC 함수 (현재 사용하지 않음 - 나중에 플레이어 상호작용 추가시 사용 만약 사용 안할시 최종 버전에서 제거할 예정)
	// void ServerSetBarkingState(bool bNewBarking);

	
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

	

protected:
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
};
