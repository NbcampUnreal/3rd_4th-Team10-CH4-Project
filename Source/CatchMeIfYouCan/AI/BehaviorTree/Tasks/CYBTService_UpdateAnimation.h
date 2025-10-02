#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CYBTService_UpdateAnimation.generated.h"

/**
 * 이 서비스가 활성화된 동안, AI 캐릭터의 애니메이션 변수를 주기적으로 업데이트합니다.
 * 'Move To'와 같이 애니메이션을 직접 제어하지 않는 태스크와 함께 사용됩니다.
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYBTService_UpdateAnimation : public UBTService
{
	GENERATED_BODY()

public:
	UCYBTService_UpdateAnimation();

protected:
	/** 서비스가 활성화된 동안 주기적으로 호출되는 함수입니다. */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	/** 애니메이션에 설정할 속도 값입니다. (예: 걷기=150, 뛰기=300) */
	UPROPERTY(EditAnywhere, Category = "AI")
	float AnimationSpeed = 300.0f;
};
