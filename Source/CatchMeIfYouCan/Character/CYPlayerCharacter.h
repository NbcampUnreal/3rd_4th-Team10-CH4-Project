// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYCharacterBase.h"
#include "CYPlayerCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class UCYInputConfig;
class UInputMappingContext;

UCLASS()
class CATCHMEIFYOUCAN_API ACYPlayerCharacter : public ACYCharacterBase
{
	GENERATED_BODY()

public:
	ACYPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void PawnClientRestart() override;

	// 폰이 컨트롤러에 의해 possessed 될 때 서버에서만 호출되는 함수
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);

	void Input_AbilityInputTagStarted(FGameplayTag InputTag);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	// Client측 ASC초기화 등 수행
	virtual void OnRep_PlayerState() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CY|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CY|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

private:
	UPROPERTY(EditDefaultsOnly, Category = "CY|Input")
	TObjectPtr<UCYInputConfig> DefaultInputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "CY|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
