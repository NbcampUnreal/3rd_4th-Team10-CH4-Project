#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYLobbyGameMode.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API ACYLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

};
