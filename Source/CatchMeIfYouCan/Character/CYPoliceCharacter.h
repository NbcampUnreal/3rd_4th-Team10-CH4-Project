// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYPlayerCharacter.h"
#include "CYPoliceCharacter.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYPoliceCharacter : public ACYPlayerCharacter
{
	GENERATED_BODY()

public:

	ACYPoliceCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	
	virtual void BeginPlay() override;

};
