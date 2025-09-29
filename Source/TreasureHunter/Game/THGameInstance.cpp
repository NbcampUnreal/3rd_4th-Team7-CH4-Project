// Fill out your copyright notice in the Description page of Project Settings.


#include "THGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineSessionInterface.h"


const FName UTHGameInstance::kSessionName(TEXT("GameSession"));

void UTHGameInstance::Init()
{
	Super::Init();
	OSS = IOnlineSubsystem::Get();
	if (!OSS) { UE_LOG(LogTemp, Error, TEXT("OSS not found")); return; }

	SessionInterface = OSS->GetSessionInterface();
	Identity = OSS->GetIdentityInterface();
	ExternalUI = OSS->GetExternalUIInterface();

	if (SessionInterface.IsValid())
	{
		InviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UTHGameInstance::OnSessionUserInviteAccepted));
	}

	if (Identity.IsValid())
	{
		Identity->AutoLogin(0);
	}
}

void UTHGameInstance::Shutdown()
{
	if (SessionInterface.IsValid())
	{
		if (InviteAcceptedHandle.IsValid())
			SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(InviteAcceptedHandle);
		if (FindSessionsHandle.IsValid())
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);
		if (JoinSessionHandle.IsValid())
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
		if (CreateSessionHandle.IsValid())
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
		if (StartSessionHandle.IsValid())
			SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionHandle);
	}
	Super::Shutdown();
}

void UTHGameInstance::HostListen(bool bIsLAN)
{
	if (!SessionInterface.IsValid()) return;

	if (SessionInterface->GetNamedSession(kSessionName))
	{
		SessionInterface->DestroySession(kSessionName);
	}

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = bIsLAN;
	Settings.bUsesPresence = true;
	Settings.bUseLobbiesIfAvailable = true;
	Settings.NumPublicConnections = 2;
	Settings.bAllowJoinInProgress = true;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinViaPresence = true;

	bIsHosting = true;

	if (CreateSessionHandle.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
	}
	CreateSessionHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UTHGameInstance::OnCreateSessionComplete));
	SessionInterface->CreateSession(0, kSessionName, Settings);

	OnMatchmakingProgress.Broadcast(true);
}

void UTHGameInstance::OnCreateSessionComplete(FName, bool bWasSuccessful)
{
	if (!bWasSuccessful) { UE_LOG(LogTemp, Error, TEXT("CreateSession failed")); OnMatchmakingProgress.Broadcast(false); return; }

	if (StartSessionHandle.IsValid())
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionHandle);
	}

	StartSessionHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &UTHGameInstance::OnStartSessionComplete));
	SessionInterface->StartSession(kSessionName);
}

void UTHGameInstance::OnStartSessionComplete(FName, bool bWasSuccessful)
{
	if (!bWasSuccessful) { UE_LOG(LogTemp, Error, TEXT("StartSession failed")); return; }

	TravelListen(PendingListenMap);
	OnMatchmakingProgress.Broadcast(false);
}

void UTHGameInstance::TravelListen(const FString& MapName)
{
	FString Options = FString::Printf(TEXT("?listen"));
	UGameplayStatics::OpenLevel(GetWorld(), FName(*MapName), true, Options);
}

void UTHGameInstance::FindAndJoin(bool bIsLAN)
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->bIsLanQuery = bIsLAN;
	SessionSearch->MaxSearchResults = 50;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	if (FindSessionsHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);
	}

	FindSessionsHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UTHGameInstance::OnFindSessionsComplete));
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());

	bIsHosting = false;
	OnMatchmakingProgress.Broadcast(true);
}

bool UTHGameInstance::SendInviteToFriend(const FString& FriendUniqueNetIdStr)
{
	if (!SessionInterface.IsValid()) return false;
	if (!SessionInterface->GetNamedSession(kSessionName)) return false;

	if (!Identity.IsValid()) return false;

	TSharedPtr<const FUniqueNetId> FriendId = Identity->CreateUniquePlayerId(FriendUniqueNetIdStr);
	if (!FriendId.IsValid()) return false;

	return SessionInterface->SendSessionInviteToFriend(0, kSessionName, *FriendId);
}

void UTHGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful || !SessionSearch.IsValid() || SessionSearch->SearchResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No sessions found"));
		OnMatchmakingProgress.Broadcast(false);
		return;
	}

	const int32 Count = SessionSearch->SearchResults.Num();
	const int32 Pick = FMath::RandRange(0, Count - 1);
	const FOnlineSessionSearchResult& Chosen = SessionSearch->SearchResults[Pick];

	if (JoinSessionHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
	}
	JoinSessionHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UTHGameInstance::OnJoinSessionComplete));

	SessionInterface->JoinSession(0, kSessionName, Chosen);
}

void UTHGameInstance::OnJoinSessionComplete(FName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("JoinSession failed: %d"), (int32)Result);
		OnMatchmakingProgress.Broadcast(false);
		return;
	}

	TravelClientToSession();
	OnMatchmakingProgress.Broadcast(false);
}

void UTHGameInstance::OnSessionUserInviteAccepted(bool bWasSuccessful, int32, TSharedPtr<const FUniqueNetId>, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful || !SessionInterface.IsValid()) return;

	if (JoinSessionHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
	}

	JoinSessionHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UTHGameInstance::OnJoinSessionComplete));
	SessionInterface->JoinSession(0, kSessionName, InviteResult);
}

void UTHGameInstance::TravelClientToSession()
{
	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(kSessionName, ConnectString))
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
}
