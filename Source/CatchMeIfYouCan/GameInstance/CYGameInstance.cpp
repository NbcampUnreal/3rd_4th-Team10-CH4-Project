#include "GameInstance/CYGameInstance.h"
#include "GameModes/Login/CYLoginGameMode.h"
#include "Online/OnlineAsyncOp.h"
#include "Online/Auth.h"
#include "Online/OnlineResult.h"

void UCYGameInstance::Init()
{
    Super::Init();
    
    InitializeOnlineServices();

    CallShowLoginUI();
}

void UCYGameInstance::Shutdown()
{
    // TODO: 세션 정리 등 필요 시 여기에 작성
    Super::Shutdown();
}

void UCYGameInstance::InitializeOnlineServices()
{
    OnlineServices = UE::Online::GetServices(UE::Online::EOnlineServices::Epic);
    if (!OnlineServices.IsValid())
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
                FString::Printf(TEXT("서비스 초기화 실패")));
        return;
    }

    const FString Provider = LexToString(OnlineServices->GetServicesProvider());
    if (Provider != TEXT("Epic"))
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
                    FString::Printf(TEXT("잘못된 서비스 제공자: %s"), *Provider));
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

void UCYGameInstance::CallShowLoginUI()
{
    if (!ExternalUIInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FExternalUIShowLoginUI::Params Params;
    Params.PlatformUserId = FPlatformUserId::CreateFromInternalId(0);
    Params.Scopes = { TEXT("BasicProfile") };
    
    ExternalUIInterface->ShowLoginUI(MoveTemp(Params)).OnComplete(this, &UCYGameInstance::HandleShowLoginUIComplete);
}

void UCYGameInstance::HandleShowLoginUIComplete(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result)
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
    CallQueryUserInfo();
    
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

void UCYGameInstance::CallQueryUserInfo()
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
    FString ServicesType = LexToString(LocalAccountId.GetOnlineServicesType());
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
            FString::Printf(TEXT("로그인 성공 (닉네임): %s"), *DisplayName));
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
            FString::Printf(TEXT("계정 아이디 서비스 타입: %s"), *ServicesType));
    }
}

void UCYGameInstance::CallCreateSession()
{
    if (!SessionsInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FCustomSessionSetting HostAccountIdSetting;
    HostAccountIdSetting.Data = UE::Online::FSchemaVariant(ToString(LocalAccountId)); 
    HostAccountIdSetting.Visibility = UE::Online::ESchemaAttributeVisibility::Public;
    
    UE::Online::FCustomSessionSetting SessionSearchSetting;
    SessionSearchSetting.Data = UE::Online::FSchemaVariant(TEXT("Y"));
    SessionSearchSetting.Visibility = UE::Online::ESchemaAttributeVisibility::Public;
    
    UE::Online::FCreateSession::Params Params;
    Params.LocalAccountId = LocalAccountId;
    Params.bPresenceEnabled = true;
    Params.SessionName = FName(TEXT("CYSession"));
    Params.SessionSettings.SchemaName = TEXT("DefaultSchema"); // 포털에서 스키마 지정 -> 지정된 세팅 자동 적용, 지정되지 않은 세팅 추가하거나 지정된 값과 다르게 같은 세팅을 추가하면 안 됨.)
    Params.SessionSettings.bAllowNewMembers = true;
    Params.SessionSettings.NumMaxConnections = 6;
    Params.SessionSettings.CustomSettings.Add(TEXT("HostAccountId"), MoveTemp(HostAccountIdSetting));
    Params.SessionSettings.CustomSettings.Add(TEXT("C"), MoveTemp(SessionSearchSetting));
    
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
            TEXT("CreateSession 실패. 코드: %s, 상세: %s"),
            *ErrorCodeStr,      
            *ErrorDetailStr     
        );
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, ErrorMsg);
        return;
    }
    
    CallAddSessionMember();
}

void UCYGameInstance::CallFindSessions()
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
            CallCreateSession();
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
            CallJoinSession(FoundSessions[0]);
        }
    }
}

void UCYGameInstance::CallJoinSession(const UE::Online::FOnlineSessionId& SessionIdToJoin)
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
}

void UCYGameInstance::HandleJoinSessionComplete(const UE::Online::TOnlineResult<UE::Online::FJoinSession>& Result)
{
    if (!Result.IsOk())
    {
        return;
    }

    if (!SessionsInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FGetSessionByName::Params Params;
    Params.LocalName = FName(TEXT("CYSession"));
    
    UE::Online::TOnlineResult<UE::Online::FGetSessionByName> GetSessionByNameResult = SessionsInterface->GetSessionByName(MoveTemp(Params));
    if (!GetSessionByNameResult.IsOk())
    {
        return;
    }

    JoinedSessionId = GetSessionByNameResult.GetOkValue().Session->GetSessionId();
    
    CallAddSessionMember();
}

void UCYGameInstance::CallAddSessionMember()
{
    if (!SessionsInterface.IsValid())
    {
        return;
    }
    
    UE::Online::FAddSessionMember::Params Params;
    Params.LocalAccountId = LocalAccountId;
    Params.SessionName = FName(TEXT("CYSession"));
    
    SessionsInterface->AddSessionMember(MoveTemp(Params)).OnComplete(this, &UCYGameInstance::HandleAddSessionMemberComplete);
}

void UCYGameInstance::HandleAddSessionMemberComplete(const UE::Online::TOnlineResult<UE::Online::FAddSessionMember>& Result)
{
    if (!Result.IsOk())
    {
        const UE::Online::FOnlineError& ErrorValue = Result.GetErrorValue(); 
        FString ErrorCodeStr = ErrorValue.GetErrorId(); 
        FString ErrorDetailStr = ErrorValue.GetErrorDetails()->GetText(ErrorValue).ToString(); 
        FString ErrorMsg = FString::Printf(
            TEXT("AddSessionMember 실패. 코드: %s, 상세: %s"),
            *ErrorCodeStr,      
            *ErrorDetailStr     
        );
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green, ErrorMsg);
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
                FString::Printf(TEXT("멤버 등록 실패")));
        return;
    }
    
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
                FString::Printf(TEXT("멤버 등록 성공")));
    
    if (ButtonType == EButtonType::Host)
    {
        ButtonType = EButtonType::None;
        
        GetWorld()->ServerTravel("/Game/Maps/Lobby?listen", true);
    }
    else if (ButtonType == EButtonType::Join)
    {
        ButtonType = EButtonType::None;
        
        if (!OnlineServices.IsValid())
        {
            return;
        }
        
        UE::Online::FGetResolvedConnectString::Params Params;
        Params.LocalAccountId = LocalAccountId;
        Params.SessionId = JoinedSessionId;
        Params.PortType = NAME_GamePort;

        FString StrAcoountId = ToString(Params.LocalAccountId);
        FString StrSessionId = ToString(Params.SessionId);
        FString StrPortType = Params.PortType.ToString();
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
            FString::Printf(TEXT("호출자 계정 아이디: %s"), *StrAcoountId));
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
            FString::Printf(TEXT("세션 아이디: %s"), *StrSessionId));
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
            FString::Printf(TEXT("포트 타입: %s"), *StrPortType));
        FString StrOnlineServicesProvider = LexToString(OnlineServices->GetServicesProvider());
        FString StrAccountIdType = LexToString(Params.LocalAccountId.GetOnlineServicesType());
        FString StrSessionIdType = LexToString(Params.SessionId.GetOnlineServicesType());
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
            FString::Printf(TEXT("온라인 서비스 제공자: %s"), *StrOnlineServicesProvider));
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
            FString::Printf(TEXT("계정 아이디 서비스 타입: %s"), *StrAccountIdType));
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
            FString::Printf(TEXT("세션 아이디 서비스 타입: %s"), *StrSessionIdType));
        
        UE::Online::TOnlineResult<UE::Online::FGetResolvedConnectString> GetConnectStringResult = OnlineServices->GetResolvedConnectString(MoveTemp(Params));
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
}
