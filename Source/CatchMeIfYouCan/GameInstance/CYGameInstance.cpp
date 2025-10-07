#include "GameInstance/CYGameInstance.h"
#include "GameModes/Login/CYLoginGameMode.h"
#include "Online/OnlineAsyncOp.h"
#include "Online/Auth.h"
#include "Online/OnlineResult.h"

void UCYGameInstance::Init()
{
    Super::Init();
    
    InitializeOnlineServices();

    LoginToEAS();
}

void UCYGameInstance::Shutdown()
{
    // TODO: 세션 정리 등 필요 시 여기에 작성
    Super::Shutdown();
}

void UCYGameInstance::InitializeOnlineServices()
{
    OnlineServices = UE::Online::GetServices(); 
    if (!OnlineServices.IsValid())
    {
        return;
    }

    ExternalUIInterface = OnlineServices->GetExternalUIInterface();
    if (!ExternalUIInterface.IsValid())
    {
        return;
    }
    
    UserInfoInterface = OnlineServices->GetUserInfoInterface();
    if (!UserInfoInterface.IsValid())
    {
        return;
    }
    
    SessionsInterface = OnlineServices->GetSessionsInterface();
    if (!SessionsInterface.IsValid())
    {
        return;
    }
}

void UCYGameInstance::LoginToEAS()
{
    if (!ExternalUIInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FExternalUIShowLoginUI::Params Params;
    Params.PlatformUserId = FPlatformUserId::CreateFromInternalId(0);
    Params.Scopes = { TEXT("BasicProfile") };
    
    ExternalUIInterface->ShowLoginUI(MoveTemp(Params)).OnComplete(this, &UCYGameInstance::HandleLoginToEASComplete);
}

void UCYGameInstance::HandleLoginToEASComplete(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result)
{
    if (!Result.IsOk())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
            FString::Printf(TEXT("로그인 실패")));
        }
        return;
    }
    
    LocalAccountId = Result.GetOkValue().AccountInfo->AccountId;
    QueryUserInfo();
    
    UWorld* World = GetWorld();
    if (World)
    {
        AGameModeBase* GameMode = World->GetAuthGameMode();
        ACYLoginGameMode* CYLoginGameMode = Cast<ACYLoginGameMode>(GameMode);
        if (!IsValid(CYLoginGameMode))
        {
            return;
        }

        CYLoginGameMode->ShowLoginLevel();
    }
}

void UCYGameInstance::QueryUserInfo()
{
    if (!UserInfoInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FQueryUserInfo::Params Params;
    Params.LocalAccountId = LocalAccountId;
    Params.AccountIds.Add(LocalAccountId);
    
    UserInfoInterface->QueryUserInfo(MoveTemp(Params)).OnComplete(this, &UCYGameInstance::HandleQueryUserInfoComplete);
}

void UCYGameInstance::HandleQueryUserInfoComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserInfo>& Result)
{
    if (!Result.IsOk())
    {
        return;
    }

    UE::Online::FGetUserInfo::Params Params;
    Params.LocalAccountId = LocalAccountId;
    Params.AccountId = LocalAccountId;

    const UE::Online::TOnlineResult<UE::Online::FGetUserInfo>& GetUserInfoResult = UserInfoInterface->GetUserInfo(MoveTemp(Params));

    if (!GetUserInfoResult.IsOk())
    {
        return;
    }

    const UE::Online::FUserInfo& UserInfo = *GetUserInfoResult.GetOkValue().UserInfo;
    FString DisplayName = UserInfo.DisplayName;
        
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
            FString::Printf(TEXT("로그인 성공 (닉네임): %s"), *DisplayName));
    }
}

void UCYGameInstance::CreateSession()
{
    if (!SessionsInterface.IsValid())
    {
        return;
    }

    UE::Online::FCustomSessionSetting CustomSetting;
    CustomSetting.Data = UE::Online::FSchemaVariant(TEXT("Y"));
    CustomSetting.Visibility = UE::Online::ESchemaAttributeVisibility::Public;
    
    UE::Online::FCreateSession::Params Params;
    Params.LocalAccountId = LocalAccountId;
    Params.SessionName = FName(TEXT("CYSession"));
    Params.SessionSettings.SchemaName = TEXT("DefaultSchema"); // 포털에서 스키마 지정 -> 지정된 세팅 자동 적용, 지정되지 않은 세팅 추가하거나 지정된 값과 다르게 같은 세팅을 추가하면 안 됨.)
    Params.SessionSettings.bAllowNewMembers = true;
    Params.SessionSettings.NumMaxConnections = 6;
    Params.SessionSettings.CustomSettings.Add(TEXT("C"), MoveTemp(CustomSetting));
    
    SessionsInterface->CreateSession(MoveTemp(Params)).OnComplete(this, &UCYGameInstance::HandleCreateSessionComplete);
}

void UCYGameInstance::HandleCreateSessionComplete(const UE::Online::TOnlineResult<UE::Online::FCreateSession>& Result)
{
    if (!Result.IsOk())
    {
        //onlineservices\interface\onlineerror.h
        const UE::Online::FOnlineError& ErrorValue = Result.GetErrorValue(); 

        // 2. FOnlineError::GetErrorId()를 사용하여 코드 문자열을 가져옵니다.
        FString ErrorCodeStr = ErrorValue.GetErrorId(); 

        // 3. GetErrorDetails()->GetText()에 FOnlineError 객체 자신을 인자로 전달하고 .ToString()으로 FString 변환
        FString ErrorDetailStr = ErrorValue.GetErrorDetails()->GetText(ErrorValue).ToString(); 
        
        // 4. GEngine 출력
        FString ErrorMsg = FString::Printf(
            TEXT("FindSessions 실패. 코드: %s, 상세: %s"),
            *ErrorCodeStr,      
            *ErrorDetailStr     
        );
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, ErrorMsg);
        return;
    }
    
    GetWorld()->ServerTravel("/Game/Maps/LobbyLevel?listen", true);
}

void UCYGameInstance::FindSessions()
{
    if (!SessionsInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FFindSessionsSearchFilter SearchFilter;
    SearchFilter.Key = TEXT("C");
    SearchFilter.ComparisonOp = UE::Online::ESchemaAttributeComparisonOp::Equals;
    SearchFilter.Value = TEXT("Y");
    
    UE::Online::FFindSessions::Params Params;
    Params.LocalAccountId = LocalAccountId;
    Params.Filters.Add(SearchFilter);
    Params.MaxResults = 5;

    SessionsInterface->FindSessions(MoveTemp(Params)).OnComplete(this, &UCYGameInstance::HandleFindSessionsComplete);
}

void UCYGameInstance::HandleFindSessionsComplete(const UE::Online::TOnlineResult<UE::Online::FFindSessions>& Result)
{
    if (!Result.IsOk())
    {
        return;
    }

    const TArray<UE::Online::FOnlineSessionId>& FoundSessions = Result.GetOkValue().FoundSessionIds;

    if (ButtonType == EButtonType::Host)
    {
        if (FoundSessions.Num() == 0)
        {
            CreateSession();
        }
        else if (FoundSessions.Num() > 0)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
                FString::Printf(TEXT("세션 이미 존재")));
            }
        }
    }
    else if (ButtonType == EButtonType::Join)
    {
        if (FoundSessions.Num() == 0)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
                FString::Printf(TEXT("참여할 세션 없음")));
            }
        }
        else if (FoundSessions.Num() > 0)
        {
            JoinSession(FoundSessions[0]);
        }
    }
}

void UCYGameInstance::JoinSession(const UE::Online::FOnlineSessionId& SessionIdToJoin)
{
    if (!SessionsInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FJoinSession::Params JoinParams;
    JoinParams.LocalAccountId = LocalAccountId;
    JoinParams.SessionId = SessionIdToJoin;
    JoinParams.SessionName = FName(TEXT("CYSession"));
    JoinParams.bPresenceEnabled = false;

    SessionsInterface->JoinSession(MoveTemp(JoinParams)).OnComplete(this, &UCYGameInstance::HandleJoinSessionComplete);
    JoinedSessionId = SessionIdToJoin;
}

void UCYGameInstance::HandleJoinSessionComplete(const UE::Online::TOnlineResult<UE::Online::FJoinSession>& Result)
{
    if (!Result.IsOk())
    {
        return;
    }
    
    UE::Online::FGetResolvedConnectString::Params Params;
    Params.LocalAccountId = LocalAccountId;
    Params.SessionId = JoinedSessionId;
    Params.PortType = NAME_GamePort;

    UE::Online::IOnlineServicesPtr LocalOnlineServices = UE::Online::GetServices(); 
    if (!LocalOnlineServices.IsValid())
    {
        return;
    }
    
    const UE::Online::TOnlineResult<UE::Online::FGetResolvedConnectString>& GetConnectStringResult = LocalOnlineServices->GetResolvedConnectString(MoveTemp(Params));
    if (!GetConnectStringResult.IsOk())
    {
        return;
    }
    
    FString ConnectString = GetConnectStringResult.GetOkValue().ResolvedConnectString;
    if (ConnectString.IsEmpty())
    {
        return;
    }
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!IsValid(PlayerController))
    {
        return;
    }

    PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
}

