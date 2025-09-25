#include "CYAIDogController.h"
#include "Ai/Characters/CYAIDogCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense.h"
#include "TimerManager.h"

ACYAIDogController::ACYAIDogController()
{
	// 감지 컴포넌트 생성 및 설정
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    
	if (SightConfig)
	{
		// 시야 설정값들
		SightConfig->SightRadius = 1000.0f;                    // 감지 반경
		SightConfig->LoseSightRadius = 1200.0f;               //  감지 해제 반경 
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;    //  감지 각도
		SightConfig->SetMaxAge(5.0f);                         // 감지 기억 시간
        
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		//여기서 감지될 예정
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

		// 감지 컴포넌트에 설정 적용
		AIPerceptionComponent->ConfigureSense(*SightConfig);
		AIPerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass()); 
		SetPerceptionComponent(*AIPerceptionComponent);
	}

	// 블랙보드 컴포넌트 생성
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	ControlledDog = nullptr;
}

void ACYAIDogController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 서버가 아닌경우 실행 X
	if (!HasAuthority()) return;

	// 제어할 개 캐릭터 참조 저장
	ControlledDog = Cast<ACYAIDogCharacter>(InPawn);
	if (!ControlledDog) return;

	// 감지 이벤트 콜백 함수 바인딩
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACYAIDogController::OnTargetPerceived);

	// 블랙보드 설정
	if (BehaviorTreeAsset && BehaviorTreeAsset->BlackboardAsset)
	{
		UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp);
	}
}

//행동 트리 시작
void ACYAIDogController::StartLogic()
{
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}
void ACYAIDogController::StopLogic()
{
	if (BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("Pooled"));
	}
}

//감지 변화시 호출될 함수
void ACYAIDogController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
	// 서버에서만 실행, 컨트롤할 개 유효성 검사
	if (!HasAuthority() || !ControlledDog) return;
    
	// 'thief' 태그가 있는 액터만 반응
	if (!Actor->ActorHasTag(FName("thief"))) return;

	const bool bIsSensed = Stimulus.WasSuccessfullySensed();

	if (bIsSensed)
	{
		// 새로운 대상 감지시 처리
		BlackboardComp->SetValueAsObject(FName("Target"), Actor);
        
		if (!ControlledDog->IsBarking())
		{
			// 짖기 시작
			ControlledDog->SetBarkingState(true);
			BlackboardComp->SetValueAsBool(FName("bIsBarking"), true);
		}
	}
	else
	{
		// 대상을 잃었을 때 - 다른 thief가 있는지 확인
		TArray<AActor*> PerceivedActors;
		AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

		bool bIsOtherThiefVisible = false;
		for (AActor* PerceivedActor : PerceivedActors)
		{
			if (PerceivedActor != Actor && PerceivedActor->ActorHasTag(FName("thief")))
			{
				// 다른 thief 발견시 그것을 새 타겟으로 설정
				bIsOtherThiefVisible = true;
				BlackboardComp->SetValueAsObject(FName("Target"), PerceivedActor);
				break;
			}
		}

		// 시야에 thief가 아무도 없을 때만 짖기 중단
		if (!bIsOtherThiefVisible)
		{
			ControlledDog->SetBarkingState(false);
			BlackboardComp->SetValueAsBool(FName("bIsBarking"), false);
			BlackboardComp->SetValueAsObject(FName("Target"), nullptr);
		}
	}
}


