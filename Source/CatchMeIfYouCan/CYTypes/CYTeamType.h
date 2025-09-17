#pragma once

UENUM(BlueprintType)
enum class ECYTeamRole : uint8
{
	None        UMETA(DisplayName = "None"),
	Cop			UMETA(DisplayName = "Cop"),  
	Robber		UMETA(DisplayName = "Robber")
};
