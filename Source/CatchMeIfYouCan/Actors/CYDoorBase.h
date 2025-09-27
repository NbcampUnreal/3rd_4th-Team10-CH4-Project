// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/CYWorldInteractable.h"
#include "CYDoorBase.generated.h"

class UArrowComponent;

UENUM(BlueprintType)
enum class ECYDoorState : uint8
{
	Open_Forward,
	Open_Backward,
	Close
};

UCLASS()
class CATCHMEIFYOUCAN_API ACYDoorBase : public ACYWorldInteractable
{
	GENERATED_BODY()

public:
	ACYDoorBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	// 상호작용 Info 제공
	virtual FCYInteractionInfo GetPreInteractionInfo(const FCYInteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetDoorState(ECYDoorState NewDoorState);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorStateChanged(ECYDoorState NewDoorState);
	
private:
	UFUNCTION()
	void OnRep_DoorState();

public:
	ECYDoorState GetDoorState() const { return DoorState; }
	
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_DoorState)
	ECYDoorState DoorState = ECYDoorState::Close;

	// 문이 열린 상태에 대한 상호작용 Info
	UPROPERTY(EditDefaultsOnly, Category="Info")
	FCYInteractionInfo OpenedInteractionInfo;

	// 문이 닫힌 상태에 대한 상호작용 Info
	UPROPERTY(EditDefaultsOnly, Category="Info")
	FCYInteractionInfo ClosedInteractionInfo;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> LeftMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> RightMeshComponent;
};
