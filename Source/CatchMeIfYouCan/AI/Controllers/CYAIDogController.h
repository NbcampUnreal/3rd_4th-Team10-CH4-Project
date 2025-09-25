// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "CYAIDogController.generated.h"

/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYAIDogController : public AAIController
{
	GENERATED_BODY()

public:
	ACYAIDogController();

	//풀링 로직 제어
	void StartLogic();
	void StopLogic();
	
protected:
	//빙의시 호출되는 함수
	virtual void OnPossess(APawn* InPawn) override;

public:
	//AI 감지 컴포넌트
	UPROPERTY(VisibleAnywhere, blueprintReadOnly)
	class UAIPerceptionComponent* AIPerceptionComponent;

	// 행동 트리
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	// 블랙보드 컴포넌트
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	class UBlackboardComponent* BlackboardComp;

private:
	// 대상 인식시 호출되는 콜백 함수
	UFUNCTION()
	void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);
    
	//제어할 경비견
	class ACYAIDogCharacter* ControlledDog;
};
