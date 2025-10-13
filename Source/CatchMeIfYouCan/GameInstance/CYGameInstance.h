#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Online/OnlineServices.h"
#include "Online/ExternalUI.h"
#include "Online/UserInfo.h"
#include "Online/Sessions.h"
#include "CYGameInstance.generated.h"

UENUM(BlueprintType)
enum class EButtonType : uint8
{
	None,
	Host,
	Join
};

UCLASS()
class CATCHMEIFYOUCAN_API UCYGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	UE::Online::IOnlineServicesPtr OnlineServices;
	UE::Online::IExternalUIPtr ExternalUIInterface;
	UE::Online::IUserInfoPtr UserInfoInterface; 
	UE::Online::ISessionsPtr SessionsInterface;

	UE::Online::FAccountId LocalAccountId;
	FString SessionName = TEXT("CYSession");
	UE::Online::FOnlineSessionId JoinedSessionId;

public:
	EButtonType ButtonType;

protected:
	virtual void Init() override;
	
	virtual void Shutdown() override;

private:
	void InitializeOnlineServices();
	
	void CallShowLoginUI();

	void HandleShowLoginUIComplete(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result);

public:
	void CallQueryUserInfo();
	
	void HandleQueryUserInfoComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserInfo>& Result);
	
	void CallCreateSession();

	void HandleCreateSessionComplete(const UE::Online::TOnlineResult<UE::Online::FCreateSession>& Result);

	void CallFindSessions();

	void HandleFindSessionsComplete(const UE::Online::TOnlineResult<UE::Online::FFindSessions>& Result);

	void CallJoinSession(const UE::Online::FOnlineSessionId& SessionIdToJoin);

	void HandleJoinSessionComplete(const UE::Online::TOnlineResult<UE::Online::FJoinSession>& Result);

	void CallAddSessionMember();
	
	void HandleAddSessionMemberComplete(const UE::Online::TOnlineResult<UE::Online::FAddSessionMember>& Result);
};