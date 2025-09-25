#pragma once
#include "CoreMinimal.h"
#include "CYWidgetDelegates.generated.h"

USTRUCT(BlueprintType)
struct FSkillCooldowns
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, float> Values;
};

// ===== UI 데이터 변경 감지 델리게이트 선언 =====

// Attribute 변경을 브로드캐스트하기 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewHealth);

// 인게임 정보 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamCountInfoChanged, int32, CopCount, int32, RobberCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAliveRobberCountInfoChanged, int32, AliveCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, float, RemainingSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChangedSignature, EGamePhase, NewPhase);

// 능력치 (Attribute) 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChanged, float, NewValue);

// 게임 상태 (GameState) 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamCountChanged, int32, CopCount, int32, RobberCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAliveRobberChanged, int32, AliveRobber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChanged, float, Seconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);

// 플레이어 상태 (PlayerState/Character) 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoleChanged, ECYTeamRole, NewRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJailProgressChanged, bool, bInJail, float, Progress01);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillCooldownsChanged, const FSkillCooldowns&, Cooldowns);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPromptChanged, FText, PromptText);