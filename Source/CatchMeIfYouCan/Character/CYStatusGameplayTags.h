#pragma once

#include "NativeGameplayTags.h"

namespace CYGameplayTags
{
	// 단발성 Action 상태
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Action_Jump);
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Action_AbilityInteract);

	// 지속성 Movement 상태
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Movement_Climbing);

	// AI 감지 어빌리티 이벤트 태그
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_AI_RobberDetected);
	CATCHMEIFYOUCAN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_AI_RobberLost);
}