#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "THGameModeBase.generated.h"

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
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void SetGameModeFlow(const FGameplayTag& NewPhase);

	void LoadGame();
	void GameStart();
	void InitialzationGameData();

	bool GetBunnyIsWinning() const { return bBunnyHasBeenWinning; }

	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void PlayerDetected(AActor* Player);

	void SetAfterTheGame(const FGameplayTag& AfterGameOver, ATHPlayerController* Requester);

protected:
	void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;

private:
	void GameStartPlayerControllers(ATHPlayerController* NewPlayer);
	void StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad);
	void OnLevelLoadedReady();
	void CheckPlayReady();
	void AccumulatePlayerDistance();
	void CourseCalculate();
	bool IsInFlatSection(const FVector& PlayerPos) const;
	void ReMatchGame();
	void ApplyOnlineNickname(ATHPlayerState* PlayerState);

private:
	FGameplayTag GameModeFlow;
	FGameplayTag RequestRematchState;

	int32 ServerEnterPlayerNum;
	int32 MaxMatchPlayerNum;
	int32 StartPlayerNum;

	FTimerHandle AccumulateUpdateTimerHandle;
	FTimerHandle LoadMainTimerHandle;
	FTimerHandle MatchTimerHandle;

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

	float ACRevisionValueZ;
	float TotalDist;
	float FlatSectionDist;
	float ClimbSectionDist;
	float Section1Weight;
	float Section2Weight;
	bool bBunnyHasBeenWinning;

public:
	UPROPERTY()
	TArray<APlayerController*> LoginPlayerControllers;
	UPROPERTY()
	TArray<ATHPlayerState*> EnteredPlayerStates;
	UPROPERTY()
	TArray<ATHPlayerController*> StartPlayerControllers;


#pragma region Loading
private:
	TSoftObjectPtr<UWorld> LastPlayedLevel;
	TSet<TWeakObjectPtr<AController>> LoadedControllers;

	void PrepareForTravel();

public:
	void NotifyClientLoaded(ATHPlayerController* PC);

private:
	void TryStartWhenAllReady();
#pragma endregion
};
