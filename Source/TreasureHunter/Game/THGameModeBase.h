#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
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

UCLASS()
class TREASUREHUNTER_API ATHGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ATHGameModeBase();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void SetGameModeFlow(const FGameplayTag& NewPhase);

	void StartMatchGame(ATHTitlePlayerController* PC);

	void DecidePlayCharacter();

	void WaitGame();

	void MatchGame();

	void LoadGame();

	void GameStart();

	void FinishGame();

	void ShowResult();

	void InitialzationGameData();

	void ManipluateController(bool Manipulate);

	void OpenChangeLevel(FGameplayTag NextFlow);

private:
	FGameplayTag GetGameModeFlow() const;

	void SetPlayerData(FString& Adrress, FString& UniqueId);

	bool CheckEnoughPlayer();

	void StartMatchTimer();

	void StopMatch(bool Rematch);

	void EnterTitlePlayerControllers(ATHTitlePlayerController* NewPlayer);

	void GameStartPlayerControllers(ATHPlayerController* NewPlayer);

	void StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad);

	void OnLevelLoadedReady();

	void CheckPlayReady();

private:
	FGameplayTag GameModeFlow;

	int32 ServerEnterPlayerNum;

	int32 CurMatchWaitPlayerNum;	

	int32 MaxMatchPlayerNum;

	int32 ReadyPlayerNum;

	int32 StartPlayerNum;

	FTimerHandle MatchTimerHandle;

	float MatchWaitTime;

	TSoftObjectPtr<UWorld> OpenLevelPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> MainLevelPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> PlayLevelPath;

	UPROPERTY()
	uint8 bIsLevelLoading : 1;
	UPROPERTY()
	uint8 bIsPlayer1Ready : 1;
	UPROPERTY()
	uint8 bIsPlayer2Ready : 1;

public:
	TArray<FPlayerData> LoginPlayerData;

	TArray<FPlayerData> StartPlayerData;

	TArray<APlayerController*> LoginPlayerControllers;

	TArray<ATHTitlePlayerController*> MatchWaitPlayerControllers;

	TArray<ATHTitlePlayerController*> MatchPlayerControllers;

	TArray<ATHPlayerController*> StartPlayerControllers;
};
