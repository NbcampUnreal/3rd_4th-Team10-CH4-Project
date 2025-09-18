// Fill out your copyright notice in the Description page of Project Settings.


#include "CYPawnData.h"

UClass* UCYPawnData::LoadPawnClass() const
{
	return PawnClass.IsValid() ? PawnClass.LoadSynchronous() : nullptr;
}

UTexture2D* UCYPawnData::LoadPawnIcon() const
{
	return PawnIcon.IsValid() ? PawnIcon.LoadSynchronous() : nullptr;
}

