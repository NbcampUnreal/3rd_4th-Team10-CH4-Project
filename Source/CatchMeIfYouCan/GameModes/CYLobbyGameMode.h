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

private:
	void QueryPlayerNickname(APlayerController* NewPlayer);
	
	void OnQueryUserInfoComplete(int32 LocalUserNum,
								bool bWasSuccessful,
								const TArray<TSharedRef<const FUniqueNetId>>& UserIds,
								const FString& ErrorStr);
};
