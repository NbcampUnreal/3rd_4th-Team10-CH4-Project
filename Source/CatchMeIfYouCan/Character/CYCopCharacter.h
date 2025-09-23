// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYPlayerCharacter.h"
#include "CYCopCharacter.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYCopCharacter : public ACYPlayerCharacter
{
	GENERATED_BODY()

public:

	ACYCopCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	
	virtual void BeginPlay() override;

};
