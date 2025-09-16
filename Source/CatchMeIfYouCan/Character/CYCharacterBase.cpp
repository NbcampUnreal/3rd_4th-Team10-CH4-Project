#include "CYCharacterBase.h"

#include "CYLogChannels.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"

ACYCharacterBase::ACYCharacterBase(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	
}

UAbilitySystemComponent* ACYCharacterBase::GetAbilitySystemComponent() const
{
	return CYAbilitySystemComponent.Get();
}

void ACYCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACYCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAbilitySets();
	Super::EndPlay(EndPlayReason);
}

void ACYCharacterBase::InitializeAbilitySets()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (!CYAbilitySystemComponent.IsValid())
	{
		UE_LOG(LogCY, Warning, TEXT("%s Side: InitializeAbilitySets: AbilitySystemComponent is invalid on %s"), *GetClientServerContextString(this), *GetNameSafe(this));
		return;
	}

	// 기존 Ability 제거
	RemoveAbilitySets();

	UCYAbilitySystemComponent* CYASC = CYAbilitySystemComponent.Get();
	
	// 기본적으로 부여할 AbilitySet 설정
	for (UCYAbilitySet* AbilitySet : DefaultAbilitySets)
	{
		if (IsValid(AbilitySet))
		{
			// 해당 시점에서 AbilitySet을 부여하고 부여된 Ability들의 Handle을 GrantedAbilitySetHandles에 저장
			FCYAbilitySet_GrantedHandles& GrantedHandles = GrantedAbilitySetHandles.AddDefaulted_GetRef();
			AbilitySet->GiveToAbilitySystem(CYASC, &GrantedHandles);
		}
	}
}

void ACYCharacterBase::RemoveAbilitySets()
{
	// 서버에서만 제거
	if (!HasAuthority())
	{
		return;
	}
	
	if (!CYAbilitySystemComponent.IsValid())
	{
		return;
	}

	UCYAbilitySystemComponent* CYASC = CYAbilitySystemComponent.Get();

	// 모든 부여된 능력 제거
	for (FCYAbilitySet_GrantedHandles& GrantedHandles : GrantedAbilitySetHandles)
	{
		GrantedHandles.TakeFromAbilitySystem(CYASC);
	}
	GrantedAbilitySetHandles.Empty();
}


