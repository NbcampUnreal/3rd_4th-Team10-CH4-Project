// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYPlayerCharacter.h"
#include "CYRobberCharacter.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYRobberCharacter : public ACYPlayerCharacter
{
	GENERATED_BODY()

public:

	ACYRobberCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void BeginPlay() override;

};
