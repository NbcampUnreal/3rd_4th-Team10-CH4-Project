#pragma once
#include "CYWidgetDelegates.generated.h"

// UENUM(BlueprintType)인 열거형 사용 for BP
UENUM(BlueprintType)
enum class ECYGamePhase : uint8 { Lobby, Loading, InProgress, Ending };

UENUM(BlueprintType)
enum class ECYTeamRole : uint8 { Cop, Robber };

// ===== UI 데이터 변경 감지 델리게이트 선언 =====

// Attribute 변경을 브로드캐스트하기 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewHealth);

// 인게임 정보 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamCountInfoChanged, int32, CopCount, int32, RobberCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAliveRobberCountInfoChanged, int32, AliveCount);

// 능력치 (Attribute) 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChanged, float, NewValue);

// 게임 상태 (GameState) 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamCountChanged, int32, CopCount, int32, RobberCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAliveRobberChanged, int32, AliveRobber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChanged, float, Seconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, ECYGamePhase, NewPhase);

// 플레이어 상태 (PlayerState/Character) 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoleChanged, ECYTeamRole, NewRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJailProgressChanged, bool, bInJail, float, Progress01);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillCooldownsChanged, TMap<FName, float>, Cooldowns);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPromptChanged, FText, PromptText);