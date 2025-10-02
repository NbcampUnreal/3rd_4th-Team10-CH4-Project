#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "CYAnimNotify_WeaponAttack.generated.h"

UCLASS(BlueprintType, Blueprintable)
class CATCHMEIFYOUCAN_API UCYAnimNotify_WeaponAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	UCYAnimNotify_WeaponAttack();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	// 공격 이벤트 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEvent")
	FGameplayTag AttackEventTag;
};