#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "THGameInstance.generated.h"

UCLASS()
class TREASUREHUNTER_API UTHGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
	void HostSession(FName SessionName, int32 MaxPlayers, const FString& PublicIP, int32 Port);

	void JoinServer(const FString& ServerAddress);
	
#pragma region Session
private:
	FName JoinSessionInfo;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	IOnlineSessionPtr SessionInterface;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
		
	void OnFindSessionsComplete(bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
#pragma endregion

#pragma region Server Travel
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> MainLevelPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> PlayLevelPath;
#pragma endregion
};
