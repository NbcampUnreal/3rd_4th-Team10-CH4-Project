#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CYGameInstance.generated.h"

UCLASS()
class CATCHMEIFYOUCAN_API UCYGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	FName CurrentSessionName;
	
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	
protected:
	virtual void Init() override;
	
	virtual void Shutdown() override;
	
public:
	IOnlineSubsystem* OSS;

	IOnlineIdentityPtr Identity;

	IOnlineSessionPtr Sessions;

	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	void OnFindSessionsComplete(bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
	void CreateSession();

	void FindSessions();

	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
};
