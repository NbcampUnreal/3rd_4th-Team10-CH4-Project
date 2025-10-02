#include "GameModes/CYGameInstance.h"
#include "OnlineSubsystemUtils.h"
//#include "Interfaces/OnlineIdentityInterface.h" (Utils에 포함)
#include "OnlineSessionSettings.h"

void UCYGameInstance::Init()
{
	Super::Init();
	
	OSS = Online::GetSubsystem(GetWorld());
	if (!OSS)
	{
		return;
	}
	
	Identity = OSS->GetIdentityInterface();
	if (Identity.IsValid())
	{
		FOnlineAccountCredentials Credentials;
		Credentials.Type = TEXT("epic");
		Credentials.Id = TEXT("");   
		Credentials.Token = TEXT(""); 

		Identity->OnLoginCompleteDelegates->AddUObject(this, &UCYGameInstance::OnLoginComplete);
		Identity->Login(0, Credentials);
	}
	else
	{
	}

	Sessions = OSS->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}

	Sessions->OnCreateSessionCompleteDelegates.AddUObject(this, &UCYGameInstance::OnCreateSessionComplete);
	Sessions->OnFindSessionsCompleteDelegates.AddUObject(this, &UCYGameInstance::OnFindSessionsComplete);
	Sessions->OnJoinSessionCompleteDelegates.AddUObject(this, &UCYGameInstance::OnJoinSessionComplete);

	SearchSettings = MakeShareable(new FOnlineSessionSearch());
	SearchSettings->bIsLanQuery = false;           
	SearchSettings->MaxSearchResults = 5;
	SearchSettings->QuerySettings.Set(FName(TEXT("PRESENCE")), true, EOnlineComparisonOp::Equals);
	SearchSettings->QuerySettings.Set(FName(TEXT("SEARCHKEYWORDS")), FString("Lobby"), EOnlineComparisonOp::Equals);
}

void UCYGameInstance::Shutdown()
{
	IOnlineSubsystem* LocalOSS = Online::GetSubsystem(GetWorld());

	if (LocalOSS)
	{
		IOnlineSessionPtr LocalSessions = LocalOSS->GetSessionInterface();

		if (LocalSessions.IsValid() && LocalSessions->GetNamedSession(CurrentSessionName))
		{
			OnDestroySessionCompleteDelegate.BindUObject(this, &UCYGameInstance::OnDestroySessionComplete);
			LocalSessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			LocalSessions->DestroySession(CurrentSessionName); 
		}
	}
	
	Super::Shutdown();
}

void UCYGameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
				FString::Printf(TEXT("EAS 로그인 성공: %s"), *UserId.ToString()));
		}

		IOnlineSubsystem* LocalOSS = Online::GetSubsystem(GetWorld());
		if (!LocalOSS) return;

		IOnlineUserPtr UserInterface = LocalOSS->GetUserInterface();
		if (!UserInterface.IsValid())
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
					FString::Printf(TEXT("유저인터페이스 가져오기 실패")));
			}
		}
	}
	else
	{
		if (GEngine)
		{
			// 프로그램 종료하면서 로그인 하라는 알림창 띄워주면 좋을 듯.
			// 로그인 창을 다시 띄워준다거나?
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
				FString::Printf(TEXT("로그인 실패: %s"), *Error));
		}
	}
}

void UCYGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel("/Game/Maps/LobbyLevel?listen", true);
	}
}

void UCYGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* LocalOSS = Online::GetSubsystem(GetWorld());

	if (LocalOSS)
	{
		IOnlineSessionPtr LocalSessions = LocalOSS->GetSessionInterface();

		if (LocalSessions.IsValid() && bWasSuccessful)
		{
			FDelegateHandle DelegateHandleToClear = OnDestroySessionCompleteDelegate.GetHandle();
			LocalSessions->ClearOnDestroySessionCompleteDelegate_Handle(DelegateHandleToClear);
		}
	}
}

void UCYGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (SearchSettings.IsValid())
		{
			if (ButtonType == EButtonType::Host)
			{
				if (SearchSettings->SearchResults.Num() > 0)
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("세션 이미 존재")));
					}
				}
				else
				{
					CreateSession();
				}
			}
			else if (ButtonType == EButtonType::Join)
			{
				if (SearchSettings->SearchResults.Num() > 0)
				{
					JoinSession(SearchSettings->SearchResults[0]);
				}
				else
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("참여할 세션 없음")));
					}
				}
			}
		}
	}
}

void UCYGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success && Sessions.IsValid())
	{
		FString TravelURL;
		if (Sessions->GetResolvedConnectString(SessionName, TravelURL))
		{
			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				PC->ClientTravel(TravelURL, TRAVEL_Absolute);
			}
		}
	}
}

void UCYGameInstance::CreateSession()
{
	if (!Sessions.IsValid()) return;
	
	CurrentSessionName = "CYSession";
	
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;            
	SessionSettings.NumPublicConnections = 6;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.Set(FName("SEARCHKEYWORDS"),
						FString("Lobby"),
						EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName(TEXT("SESSION_JOIN_NAME_KEY")), 
						CurrentSessionName.ToString(), 
						EOnlineDataAdvertisementType::ViaOnlineService);
	
	Sessions->CreateSession(0, CurrentSessionName, SessionSettings);
}

void UCYGameInstance::FindSessions()
{
	if (Sessions.IsValid() && SearchSettings.IsValid())
	{
		Sessions->FindSessions(0, SearchSettings.ToSharedRef());
	}
}

void UCYGameInstance::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (Sessions.IsValid())
	{
		FString JoinSessionNameStr;
    
		const FOnlineSessionSettings& Settings = SearchResult.Session.SessionSettings;
		
		if (Settings.Get(FName(TEXT("SESSION_JOIN_NAME_KEY")), JoinSessionNameStr))
		{
			FName SessionName = FName(*JoinSessionNameStr);
			
			Sessions->JoinSession(0, SessionName, SearchResult);
		}
	}
}
