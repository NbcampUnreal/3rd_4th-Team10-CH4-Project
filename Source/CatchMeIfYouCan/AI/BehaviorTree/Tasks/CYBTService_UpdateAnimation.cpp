#include "CYBTService_UpdateAnimation.h"
#include "AIController.h"
#include "AI/Characters/CYAIDogCharacter.h" // 경비견 캐릭터 헤더 포함
#include "BehaviorTree/BlackboardComponent.h"

UCYBTService_UpdateAnimation::UCYBTService_UpdateAnimation()
{
	NodeName = "Update Animation Variables";

	// 이 서비스가 매 프레임 실행되도록 설정합니다.
	// 좀 더 최적화하려면 Interval을 0.1초 정도로 설정해도 좋습니다.
	Interval = 0.0f; 
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UCYBTService_UpdateAnimation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		// 제어 중인 폰을 경비견 캐릭터로 캐스팅합니다.
		ACYAIDogCharacter* DogCharacter = Cast<ACYAIDogCharacter>(AIController->GetPawn());
		if (DogCharacter)
		{
			// 이동 중이므로, 설정된 속도 값으로 애니메이션 변수를 업데이트합니다.
			// 방향은 정면을 의미하는 0.0f로 설정합니다.
			DogCharacter->UpdateAIAnimationVariables(AnimationSpeed, 0.0f);
		}
	}
}
