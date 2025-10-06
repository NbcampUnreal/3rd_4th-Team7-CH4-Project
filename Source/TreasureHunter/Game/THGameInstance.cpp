#include "Game/THGameInstance.h"

#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UTHGameInstance::Init()
{
	Super::Init();

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();

        if (!SessionInterface.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("SessionInterface is nullptr! Check OnlineSubsystem config."));
        }
    }
}

void UTHGameInstance::HostSession(FName SessionName, int32 MaxPlayers, const FString& PublicIP, int32 Port)
{
    if (!SessionInterface.IsValid()) return;

    if (SessionInterface->GetNamedSession(SessionName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Session already exists: %s"), *SessionName.ToString());
        return;
    }

    SessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);

    FOnlineSessionSettings Settings;
    Settings.bIsLANMatch = false;
    Settings.NumPublicConnections = MaxPlayers;
    Settings.bShouldAdvertise = true;
    Settings.bUsesPresence = true;
    Settings.bAllowJoinInProgress = true;

    UE_LOG(LogTemp, Warning, TEXT("Creating Dedicated Server Session..."));

    SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UTHGameInstance::OnCreateSessionComplete);

    if (!SessionInterface->CreateSession(0, SessionName, Settings))
    {
        UE_LOG(LogTemp, Warning, TEXT("HostSession: Failed to start CreateSession call"));
    }

    UWorld* World = GetWorld();
    if (World)
    {
        const FString MapPath = MainLevelPath.ToSoftObjectPath().GetLongPackageName();
        FString MapURL = FString::Printf(TEXT("%s?listen&PublicIP=%s&Port%d"), *MapPath, *PublicIP, Port);
        World->ServerTravel(MapURL, true);
    }
}

void UTHGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("Session Created: %s, Success=%d"), *SessionName.ToString(), bWasSuccessful);
}

void UTHGameInstance::JoinServer(const FString& ServerAddress)
{
    if (!SessionInterface.IsValid()) return;
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = 20;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    SessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
    SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UTHGameInstance::OnFindSessionsComplete);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());

    if (ServerAddress.IsEmpty()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        FString CurrentURL = PC->GetWorld()->GetAddressURL();
        if (CurrentURL.Contains(ServerAddress))
        {
            return;
        }

        FString ConnectURL = FString::Printf(TEXT("%s?listen"), *ServerAddress);
        PC->ClientTravel(ConnectURL, TRAVEL_Absolute);
    }
}

void UTHGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
    }

    if (!bWasSuccessful || !SessionSearch.IsValid()) return;

    if (SessionSearch->SearchResults.Num() > 0)
    {
        const FOnlineSessionSearchResult& ChosenResult = SessionSearch->SearchResults[0];
        SessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
        SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UTHGameInstance::OnJoinSessionComplete);
        SessionInterface->JoinSession(0, FName(*ChosenResult.GetSessionIdStr()), ChosenResult);
    }
}

void UTHGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface.IsValid()) return;

    FString ConnectInfo;
    if (SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
    {
        if (JoinSessionInfo == SessionName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Equal Session return"));
            return;
        }

        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            JoinSessionInfo = SessionName;
            UE_LOG(LogTemp, Warning, TEXT("Connect String Resolved: %s"), *ConnectInfo);
            PC->ClientTravel(ConnectInfo, TRAVEL_Absolute);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not get connect info"));
    }
}