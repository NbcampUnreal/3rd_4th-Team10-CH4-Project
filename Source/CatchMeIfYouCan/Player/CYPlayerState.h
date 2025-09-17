// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CYTypes/CYTeamType.h"
#include "GameFramework/PlayerState.h"
#include "CYPlayerState.generated.h"


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

protected:
	UFUNCTION()
	void OnRep_TeamRole();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "CY|PlayerState")
	TObjectPtr<UCYAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UCYVitalSet> VitalSet;

	UPROPERTY(ReplicatedUsing = OnRep_TeamRole, BlueprintReadOnly, Category = "CY|Team", Meta = (AllowPrivateAccess = true))
	ECYTeamRole TeamRole = ECYTeamRole::None;
};
