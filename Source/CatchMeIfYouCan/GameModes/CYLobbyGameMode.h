#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYLobbyGameMode.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACYLobbyGameMode();
	
protected:
	virtual void BeginPlay() override;
};
