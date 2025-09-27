// Fill out your copyright notice in the Description page of Project Settings.


#include "CYGameplayAbility_Interact_Door.h"

#include "Actors/CYDoorBase.h"
#include "Kismet/KismetMathLibrary.h"


UCYGameplayAbility_Interact_Door::UCYGameplayAbility_Interact_Door()
{
}

void UCYGameplayAbility_Interact_Door::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr || bInitialized == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 문 상태 변경은 서버에서만 가능 
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 문 액터 유효성 확인
	ACYDoorBase* DoorActor = Cast<ACYDoorBase>(InteractableActor);
	if (DoorActor == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}
	
	ECYDoorState CurrentDoorState = DoorActor->GetDoorState();
	if (CurrentDoorState == ECYDoorState::Open_Forward || CurrentDoorState == ECYDoorState::Open_Backward)
	{
		// 문이 열려있으면 → 닫기
		DoorActor->SetDoorState(ECYDoorState::Close);
	}
	else
	{
		// 문이 닫혀있으면 → 플레이어 위치에 따라 방향 결정하여 열기
		if (const AActor* Instigator = TriggerEventData->Instigator)
		{
			FVector InteractableForward = InteractableActor->GetActorForwardVector();
			FVector InteractableToInstigator = UKismetMathLibrary::Vector_Normal2D(Instigator->GetActorLocation() - InteractableActor->GetActorLocation());
			float Dot = FVector::DotProduct(InteractableForward, InteractableToInstigator);

			// Dot < 0: 플레이어가 문의 뒤쪽에 있음 → 뒤쪽으로 열기
			// Dot >= 0: 플레이어가 문의 앞쪽에 있음 → 앞쪽으로 열기
			DoorActor->SetDoorState(Dot < 0.f ? ECYDoorState::Open_Backward : ECYDoorState::Open_Forward);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
