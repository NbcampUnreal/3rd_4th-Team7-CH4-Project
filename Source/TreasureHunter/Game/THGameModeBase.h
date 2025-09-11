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

private:
	FGameplayTag GetGameModeFlow() const;

	void SetPlayerData(FString& Adrress, FString& UniqueId);

	bool CheckEnoughPlayer();

private:
	FGameplayTag GameModeFlow;

	int32 EnterPlayerNum;

	int32 MaxMatchPlayerNum;

	int32 CurMatchWaitPlayerNum;	

public:
	TArray<FPlayerData> LoginPlayerData;

	TArray<APlayerController*> LoginPlayerControllers;

	TArray<ATHTitlePlayerController*> MatchWaitPlayerControllers;

	TArray<ATHTitlePlayerController*> MatchPlayerControllers;
};
