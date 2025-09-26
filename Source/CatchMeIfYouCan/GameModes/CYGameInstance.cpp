#include "GameModes/CYGameInstance.h"
#include "OnlineSubsystemUtils.h"
//#include "Interfaces/OnlineIdentityInterface.h"
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
		// 로컬 환경에서는 생략
		//Identity->Login(0, FOnlineAccountCredentials());
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
	SearchSettings->bIsLanQuery = true;           
	SearchSettings->MaxSearchResults = 5;
	SearchSettings->QuerySettings.Set(FName(TEXT("SEARCHKEYWORDS")), FString("Lobby"), EOnlineComparisonOp::Equals);
}

void UCYGameInstance::Shutdown()
{
	IOnlineSubsystem* LocalOSS = IOnlineSubsystem::Get(TEXT("NULL"));

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

void UCYGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel("/Game/Maps/LobbyLevel?listen", true);
	}
}

void UCYGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* LocalOSS = IOnlineSubsystem::Get(TEXT("NULL"));

	if (LocalOSS)
	{
		IOnlineSessionPtr LocalSessions = LocalOSS->GetSessionInterface();

		if (LocalSessions.IsValid() && bWasSuccessful)
		{
			FDelegateHandle DelegateHandleToClear = OnDestroySessionCompleteDelegate.GetHandle();
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DelegateHandleToClear);
		}
	}
}

void UCYGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && SearchSettings.IsValid() && SearchSettings->SearchResults.Num() > 0)
	{
		JoinSession(SearchSettings->SearchResults[0]);
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
	SessionSettings.bIsLANMatch = true;            
	SessionSettings.NumPublicConnections = 6;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
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
		
		//FName SessionName = FName(*SearchResult.GetSessionIdStr()); 
		//Sessions->JoinSession(0, SessionName, SearchResult);
	}
}
