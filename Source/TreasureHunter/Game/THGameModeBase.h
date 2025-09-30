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

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

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

	bool GetBunnyIsWinning() const;

	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void PlayerDetected(AActor* Player);

private:
	FGameplayTag GetGameModeFlow() const;

	void SetPlayerData(FString& Adrress, FString& UniqueId);

	bool CheckEnoughPlayer();

	void StartMatchTimer();

	void StopMatch(bool Rematch);

	void EnterTitlePlayerControllers(ATHTitlePlayerController* NewPlayer);

	void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;

	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	void GameStartPlayerControllers(ATHPlayerController* NewPlayer);

	void StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad);

	void OnLevelLoadedReady();

	void CheckPlayReady();

	void AccumulatePlayerDistance();

	void CourseCalculate();

	bool IsInFlatSection(const FVector& PlayerPos) const;

private:
	FGameplayTag GameModeFlow;

	int32 ServerEnterPlayerNum;

	int32 CurMatchWaitPlayerNum;	

	int32 MaxMatchPlayerNum;

	int32 ReadyPlayerNum;

	int32 StartPlayerNum;

	FTimerHandle MatchTimerHandle;

	FTimerHandle AccumulateUpdateTimerHandle;

	float MatchWaitTime;

	TSoftObjectPtr<UWorld> OpenLevelPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> MainLevelPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelPath", Meta = (AllowPrivateAccess))
	TSoftObjectPtr<UWorld> PlayLevelPath;

	bool bIsPlayer1Ready;
	bool bIsPlayer2Ready;

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

public:
	TArray<FPlayerData> LoginPlayerData;

	TArray<FPlayerData> StartPlayerData;

	TArray<APlayerController*> LoginPlayerControllers;

	TArray<ATHPlayerState*> EnteredPlayerStates;

	TArray<ATHTitlePlayerController*> MatchWaitPlayerControllers;

	TArray<ATHTitlePlayerController*> MatchPlayerControllers;

	TArray<ATHPlayerController*> StartPlayerControllers;
};
