// Fill out your copyright notice in the Description page of Project Settings.


#include "CYPawnData.h"

UTexture2D* UCYPawnData::LoadPawnIcon() const
{
	return PawnIcon.IsValid() ? PawnIcon.LoadSynchronous() : nullptr;
}

