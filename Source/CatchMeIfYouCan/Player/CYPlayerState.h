// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CYTypes/CYTeamType.h"
#include "GameFramework/PlayerState.h"
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
	
	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "CY|Team")
	ECYTeamRole GetTeamRole() const { return TeamRole; }
    
	UFUNCTION(BlueprintCallable, Category = "CY|Team")
	void SetTeamRole(ECYTeamRole NewTeamRole);

	UFUNCTION(BlueprintCallable, Category = "CY|PawnData")
	UCYPawnData* GetPawnData() const { return PawnData; }

	UFUNCTION(BlueprintCallable, Category = "CY|PawnData")
	void SetPawnData(UCYPawnData* NewPawnData);

protected:
	UFUNCTION()
	void OnRep_TeamRole();

	UFUNCTION()
	void OnRep_PawnData();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "CY|PlayerState")
	TObjectPtr<UCYAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UCYVitalSet> VitalSet;

	UPROPERTY(ReplicatedUsing = OnRep_TeamRole, BlueprintReadOnly, Category = "CY|Team", Meta = (AllowPrivateAccess = true))
	ECYTeamRole TeamRole = ECYTeamRole::None;

	UPROPERTY(ReplicatedUsing = OnRep_PawnData, BlueprintReadOnly, Category = "CY|PawnData", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UCYPawnData> PawnData;
};
