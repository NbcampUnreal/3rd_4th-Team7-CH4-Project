// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AdvancedFriendsGameInstance.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "BlueprintDataDefinitions.h"
#include "THGameInstance.generated.h"

class IOnlineSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingProgress, bool, bInProgress);

UCLASS()
class UTHGameInstance : public UAdvancedFriendsGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable)
    void HostListen(bool bIsLAN);
    UFUNCTION(BlueprintCallable)
    void FindAndJoin(bool bIsLAN);

    UFUNCTION(BlueprintCallable, Category = "Friends")
    bool SendInviteToFriend(const FString& FriendUniqueNetIdStr);

    UPROPERTY(BlueprintAssignable) 
    FOnMatchmakingProgress OnMatchmakingProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsHosting = false;
    static const FName kSessionName;


    UPROPERTY(EditDefaultsOnly)
    FString PendingListenMap = TEXT("StartLevel");

private:
    void TravelListen(const FString& MapName);

    void OnCreateSessionComplete(FName, bool);
    void OnStartSessionComplete(FName, bool);
    void OnFindSessionsComplete(bool);
    void OnJoinSessionComplete(FName, EOnJoinSessionCompleteResult::Type);
    void OnSessionUserInviteAccepted(bool, int32, TSharedPtr<const FUniqueNetId>, const FOnlineSessionSearchResult&);

    void TravelClientToSession();

private:
    IOnlineSubsystem* OSS = nullptr;
    IOnlineSessionPtr SessionInterface;
    IOnlineIdentityPtr Identity;
    IOnlineExternalUIPtr ExternalUI;

    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    FDelegateHandle InviteAcceptedHandle;
    FDelegateHandle FindSessionsHandle;
    FDelegateHandle JoinSessionHandle;
    FDelegateHandle CreateSessionHandle;
    FDelegateHandle StartSessionHandle;
};