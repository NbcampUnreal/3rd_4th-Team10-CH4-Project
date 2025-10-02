// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CYTypes/CYInGameTypes.h"
#include "GameFramework/PlayerState.h"
#include "UI/WidgetController/CYWidgetDelegates.h"
#include "CYPlayerState.generated.h"


class UCYPawnData;
class UCYVitalSet;
class UCYAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API ACYPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACYPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "CY|PlayerState")  
	UCYAbilitySystemComponent* GetCYAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "CY|Attributes")
	UCYVitalSet* GetVitalSet() const { return VitalSet; }
	
	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "CY|Team")
	ECYTeamRole GetTeamRole() const { return TeamRole; }

	void SetTeamRole(ECYTeamRole NewTeamRole);

	UFUNCTION(BlueprintCallable, Category = "CY|PawnData")
	UCYPawnData* GetPawnData() const { return PawnData; }

	void SetPawnData(UCYPawnData* NewPawnData);

	// Team Role Change BroaCast
	FOnTeamRoleChanged FOnTeamRoleChanged;

	// For Seamless Travel
	virtual void CopyProperties(APlayerState* PlayerState) override;

protected:
	UFUNCTION()
	void OnRep_TeamRole();

	UFUNCTION()
	void OnRep_PawnData();

private:
	void NotifyControllerPawnDataReady();
	
	UPROPERTY(VisibleAnywhere, Category = "CY|PlayerState")
	TObjectPtr<UCYAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UCYVitalSet> VitalSet;

	UPROPERTY(ReplicatedUsing = OnRep_TeamRole)
	ECYTeamRole TeamRole = ECYTeamRole::None;

	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<UCYPawnData> PawnData;
};
