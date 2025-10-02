#include "GameModes/CYLobbyGameMode.h"
#include "Player/CYPlayerState.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineUserInterface.h"

void ACYLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	QueryPlayerNickname(NewPlayer);
}

void ACYLobbyGameMode::QueryPlayerNickname(APlayerController* NewPlayer)
{
	IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
	if (!OSS) return;

	IOnlineUserPtr UserInterface = OSS->GetUserInterface();
	if (!UserInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
				FString::Printf(TEXT("유저인터페이스 가져오기 실패")));
		}
		return;
	}

	if (ACYPlayerState* CYPS = NewPlayer->GetPlayerState<ACYPlayerState>())
	{
		const FUniqueNetIdRepl& PlayerUniqueNetIdRepl = CYPS->GetUniqueId();
		const TSharedPtr<const FUniqueNetId> PlayerUniqueNetId = PlayerUniqueNetIdRepl.GetUniqueNetId();
		
		if (PlayerUniqueNetId.IsValid())
		{
			FOnQueryUserInfoCompleteDelegate Delegate = FOnQueryUserInfoCompleteDelegate::CreateUObject(this, &ACYLobbyGameMode::OnQueryUserInfoComplete);
			UserInterface->AddOnQueryUserInfoCompleteDelegate_Handle(0, Delegate);
			
			TArray<FUniqueNetIdRef> UserIds;
			UserIds.Add(PlayerUniqueNetId.ToSharedRef());

			UserInterface->QueryUserInfo(0, UserIds);
		}
	}
}

void ACYLobbyGameMode::OnQueryUserInfoComplete(int32 LocalUserNum, bool bWasSuccessful,
	const TArray<TSharedRef<const FUniqueNetId>>& UserIds, const FString& ErrorStr)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("QueryUserInfo failed: %s"), *ErrorStr);
		return;
	}

	IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
	if (!OSS) return;

	IOnlineUserPtr UserInterface = OSS->GetUserInterface();
	if (!UserInterface.IsValid()) return;

	for (auto& UserId : UserIds)
	{
		TSharedPtr<FOnlineUser> UserInfo = UserInterface->GetUserInfo(LocalUserNum, *UserId);
		if (UserInfo.IsValid())
		{
			FString DisplayName = UserInfo->GetDisplayName();

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
					FString::Printf(TEXT("닉네임 조회 성공: %s"), *DisplayName));
			}
		}
	}
}
