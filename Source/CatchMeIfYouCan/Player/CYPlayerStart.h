// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CYTypes/CYInGameTypes.h"
#include "GameFramework/PlayerStart.h"
#include "CYPlayerStart.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ACYPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	bool CanSpawnForTeam(ECYTeamRole TeamRole) const;
	
	// Occupied 상태 설정
	void SetOccupied(bool bNewOccupied) { bIsOccupied = bNewOccupied; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void UpdateEditorVisuals();
#endif

	// 이 스폰 포인트를 사용할 수 있는 팀 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CY|Team")
	ECYTeamRole AllowedTeam = ECYTeamRole::None;

	// 스폰 포인트 우선순위 (높을수록 먼저 선택)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CY|Spawn")
	int32 Priority = 0;

	// 현재 이 스폰 포인트가 사용중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "CY|Spawn")
	bool bIsOccupied = false;

};
