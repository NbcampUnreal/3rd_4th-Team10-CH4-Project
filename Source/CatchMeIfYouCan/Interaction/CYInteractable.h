#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CYInteractionInfo.h"
#include "CYInteractionQuery.h"
#include "UObject/Interface.h"
#include "CYInteractable.generated.h"

/**
 * 상호작용 정보를 수집하고 구성하는 헬퍼 클래스
 * 상호작용 가능한 객체에서 여러 개의 상호작용 정보를 생성할 때 사용
 * 각 정보에 자동으로 상호작용 객체 참조를 설정
 */
class FCYInteractionInfoBuilder
{
public:
	FCYInteractionInfoBuilder(TScriptInterface<ICYInteractable> InInteractable, TArray<FCYInteractionInfo>& InInteractionInfos)
		: Interactable(InInteractable)
		, InteractionInfos(InInteractionInfos) { }

public:
	void AddInteractionInfo(const FCYInteractionInfo& InteractionInfo)
	{
		FCYInteractionInfo& Entry = InteractionInfos.Add_GetRef(InteractionInfo);
		Entry.Interactable = Interactable; // 자동으로 상호작용 객체 참조 설정
	}
	
private:
	/** 상호작용 가능한 객체 참조 */
	TScriptInterface<ICYInteractable> Interactable;

	/** 상호작용 정보들을 저장할 배열 참조 */
	TArray<FCYInteractionInfo>& InteractionInfos;
};

UINTERFACE(MinimalAPI, BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class UCYInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용 가능한 객체들이 구현해야 하는 핵심 인터페이스
 * 문, 상자, 아이템, 캐릭터 등 모든 상호작용 가능한 객체의 기본 인터페이스 정의
 */
class ICYInteractable
{
	GENERATED_BODY()

public:
	// 상호작용 가능한 Actor에서 상속받아서 상호작용에 따른 몽타주, 부여할 어빌리티 정보를 리턴
	virtual FCYInteractionInfo GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const { return FCYInteractionInfo(); }

	/**
	 * 플레이어 스탯을 적용한 최종 상호작용 정보를 수집하는 함수
	 * GetPreInteractionInfo()에서 얻은 기본 정보에 플레이어의 GE 스탯을 적용하여
	 * 상호작용 지속시간을 조정한 후 최종 정보를 빌더에 추가
	 */
	void GatherPostInteractionInfos(const FCYInteractionQuery& InteractionQuery, FCYInteractionInfoBuilder& InteractionInfoBuilder) const
	{
		FCYInteractionInfo InteractionInfo = GetPreInteractionInfo(InteractionQuery);
	
		if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InteractionQuery.RequestingAvatar.Get()))
		{
			InteractionInfo.Duration = FMath::Max<float>(0.f, InteractionInfo.Duration);
		}
	
		InteractionInfoBuilder.AddInteractionInfo(InteractionInfo);
	}

	/**
	 * 상호작용 실행 시 게임플레이 이벤트 데이터를 커스터마이징하는 가상 함수
	 * 특정 상호작용에서 추가적인 데이터가 필요한 경우 오버라이드하여 사용
	 * @param InteractionEventTag 상호작용 이벤트 태그
	 * @param InOutEventData [입출력] 커스터마이징할 게임플레이 이벤트 데이터
	 */
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) const { }

	/**
	* 하이라이트 효과를 위한 메시 컴포넌트들을 반환하는 가상 함수
	* 상호작용 가능한 객체가 플레이어 시선에 들어올 때 아웃라인 효과를 적용하기 위해 사용
	*/
	UFUNCTION(BlueprintCallable)
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const { }

	/**
	* 현재 상황에서 상호작용이 가능한지 검증하는 가상 함수
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool CanInteraction(const FCYInteractionQuery& InteractionQuery) const { return true; }
};