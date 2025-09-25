#pragma once

UENUM(BlueprintType)
enum class ECYTeamRole : uint8
{
	None        UMETA(DisplayName = "None"),
	Cop			UMETA(DisplayName = "Cop"),  
	Robber		UMETA(DisplayName = "Robber")
};

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	WaitingToStart  UMETA(DisplayName = "Waiting To Start"),
	Preparing       UMETA(DisplayName = "Preparing"),
	InProgress      UMETA(DisplayName = "In Progress"),
	CopsWin         UMETA(DisplayName = "Cops Win"),
	RobbersWin      UMETA(DisplayName = "Robbers Win")
};