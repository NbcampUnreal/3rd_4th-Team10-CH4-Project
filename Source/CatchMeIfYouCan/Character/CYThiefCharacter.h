// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYPlayerCharacter.h"
#include "CYThiefCharacter.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYThiefCharacter : public ACYPlayerCharacter
{
	GENERATED_BODY()

public:

	ACYThiefCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void BeginPlay() override;

};
