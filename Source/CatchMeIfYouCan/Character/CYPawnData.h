#pragma once

#include "CoreMinimal.h"
#include "CYTypes/CYTeamType.h"
#include "Engine/DataAsset.h"
#include "CYPawnData.generated.h"

class UCYAbilitySet;
/**
 * 
 */
UCLASS()
class CATCHMEIFYOUCAN_API UCYPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static FPrimaryAssetType GetPawnAssetType() { return FPrimaryAssetType(TEXT("PawnData")); }

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(GetPawnAssetType(), GetFName());
	}

	// 동기 로드 (서버 스폰용)
	UClass* LoadPawnClass() const;

	UTexture2D* LoadPawnIcon() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CY|Team")
	ECYTeamRole TeamRole = ECYTeamRole::None;

	// 표기용 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CY|Pawn")
	FString PawnName;

	// 표기용 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CY|Pawn")
	TSoftObjectPtr<UTexture2D> PawnIcon;
	
	// 스폰할 Pawn 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CY|Pawn")
	TSoftClassPtr<APawn> PawnClass;

	// 이 캐릭터에 부여할 AbilitySet 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CY|Abilities")
	TArray<TObjectPtr<UCYAbilitySet>> AbilitySets;
	
	// TODO : 캐릭터 위젯등
};
