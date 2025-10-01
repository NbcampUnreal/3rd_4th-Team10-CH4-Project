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

	// 인벤토리 디버그 출력 (좌클릭)
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ShowInventoryDebug();

	//경비견 감지
	UFUNCTION(Client, Reliable)
	virtual void Client_ShowRobberDetectedWarning(bool bShow, AActor* DetectedThief);

protected:
	virtual void BeginPlay() override;

	void SetupAbilitySystemComponent();

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);

	void Input_AbilityInputTagStarted(FGameplayTag InputTag);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	// Client측 ASC초기화 등 수행
	virtual void OnRep_PlayerState() override;

	// 아이템 입력 처리
	void Input_Interact(const FInputActionValue& InputActionValue);         // E키
	void Input_Attack(const FInputActionValue& InputActionValue);           // 좌클릭
    
	// 인벤토리 슬롯 입력 (1~9번 키)
	void Input_UseSlot1(const FInputActionValue& InputActionValue);         // 무기 슬롯 1
	void Input_UseSlot2(const FInputActionValue& InputActionValue);         // 무기 슬롯 2
	void Input_UseSlot3(const FInputActionValue& InputActionValue);         // 무기 슬롯 3
	void Input_UseSlot4(const FInputActionValue& InputActionValue);         // 아이템 슬롯 1
	void Input_UseSlot5(const FInputActionValue& InputActionValue);         // 아이템 슬롯 2
	void Input_UseSlot6(const FInputActionValue& InputActionValue);         // 아이템 슬롯 3
	void Input_UseSlot7(const FInputActionValue& InputActionValue);         // 아이템 슬롯 4
	void Input_UseSlot8(const FInputActionValue& InputActionValue);         // 아이템 슬롯 5
	void Input_UseSlot9(const FInputActionValue& InputActionValue);         // 아이템 슬롯 6
	
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
