#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "THGameModeBase.generated.h"

struct FPlayerData
{
	FString PlayerAddress;
	FString PlayerName;
	FString PlayerUniqueId;
};

class UUserWidget;
class ATHTitlePlayerController;
class ATHPlayerController;
class ATHPlayerState;

UCLASS()
class TREASUREHUNTER_API ATHGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ATHGameModeBase();

	virtual void BeginPlay() override;

#pragma region PlayerController Management
private:
	int32 ServerEnterPlayerNum;

	TArray<FPlayerData> LoginPlayerData;

	TArray<ATHPlayerState*> EnteredPlayerStates;

	TArray<APlayerController*> LoginPlayerControllers;

	TArray<ATHTitlePlayerController*> MatchWaitPlayerControllers;

	TArray<ATHTitlePlayerController*> MatchPlayerControllers;

	TArray<ATHPlayerController*> StartPlayerControllers;

protected:
	virtual void PreLogin(const FString& Options, 
		const FString& Address, 
		const FUniqueNetIdRepl& UniqueId, 
		FString& ErrorMessage) override;

	virtual APlayerController* Login(UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;

	virtual void ApproveLogin(const FString& Options,
		const FString& Address,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	void SetPlayerData(FString& Adrress, FString& UniqueId);

	void ManipluateController(ATHPlayerController* Controller, bool Manipulate);

	void EnterTitlePlayerControllers(ATHTitlePlayerController* NewPlayer);

	void GameStartPlayerControllers(ATHPlayerController* NewPlayer);

	void ReconnectPlayer(ATHPlayerController* RePlayer);

#pragma endregion

#pragma region Server Travel
private:
	TSoftObjectPtr<UWorld> OpenLevelPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> MainLevelPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> PlayLevelPath;

public:
	void OpenChangeLevel(FGameplayTag NextFlow);

protected:
	void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;

	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

private:
	void OnLevelLoadedReady();

	void StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad);

#pragma endregion

#pragma region GameMode Phase
private:
	FGameplayTag GameModeFlow;

public:
	FGameplayTag GetGameModeFlow() const;

private:
	void SetGameModeFlow(const FGameplayTag& NewPhase);
#pragma endregion

#pragma region Phase Wait
private:
	int32 MaxMatchPlayerNum;

public:
	void StartMatchGame(ATHTitlePlayerController* PC);

private:
	void StartMatchWaitTimer();

	void StopMatch(bool Rematch);

	bool CheckEnoughPlayer();
#pragma endregion

#pragma region Match Phase
private:
	float MatchWaitTime;

	FTimerHandle MatchTimerHandle;

public:
	void MatchGame();

private:
#pragma endregion

#pragma region Load Phase
private:
	bool bIsPlayer1Ready;
	
	bool bIsPlayer2Ready;

public:
	void LoadGame();

private:
	void CheckPlayReady();

#pragma endregion

#pragma region Play Phase
private:
	
	TArray<FName> PosActorTags = { FName("Start"), FName("Check"), FName("Finish") };

	float ACRevisionValueZ;

	UPROPERTY()
	AActor* StartActor;

	UPROPERTY()
	AActor* CheckActor;
	
	UPROPERTY()
	AActor* FinishActor;

	FVector StartPos;

	FVector CheckPos;

	FVector FinishPos;

	float TotalDist;

	float FlatSectionDist;

	float ClimbSectionDist;

	float Section1Weight;

	float Section2Weight;

	bool bBunnyHasBeenWinning;

	FTimerHandle AccumulateUpdateTimerHandle;

public:
	bool GetBunnyIsWinning() const;

	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void PlayerDetected(AActor* Player);

private:
	void GameStart();

	void CourseCalculate();

	void AccumulatePlayerDistance();

	bool IsInFlatSection(const FVector& PlayerPos) const;
#pragma endregion

#pragma region Finish Phase
private:
	int32 RequestPlayerNum;

	FGameplayTag RequestRematchState;

	FTimerHandle LoadMainTimerHandle;

public:
	void SetAfterTheGame(const FGameplayTag& AfterGameOver, ATHPlayerController* Requester);

private:
	void ReMatchGame();
#pragma endregion


public:
	

	UPROPERTY(Transient)
	TArray<AController*> LoadedPlayers;

	int32 NumExpectedPlayers = 2;

	void NotifyClientLoaded(AController* ClientController);

protected:

	void TryStartGame();

};
