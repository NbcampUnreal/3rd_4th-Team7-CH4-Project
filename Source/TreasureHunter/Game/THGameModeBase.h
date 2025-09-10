#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/THGameModeEnum.h"
#include "THGameModeBase.generated.h"

struct FPlayerData
{
	FString PlayerName;
	FString PlayerUniqueId;
	TWeakPtr<APlayerState> PlayerState;
	TWeakPtr<APlayerController> PlayerController;
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

	void SetGameModeFlow(EGameFlow GameFlow);

	void StartMatchGame(ATHTitlePlayerController* PC);

	void DecidePlayCharacter();

	void WaitGame();

	void LoadGame();

	void GameStart();

	void FinishGame();

	void ShowResult();

	void InitialzationGameData();

	void ManipluateController(bool Manipulate);

private:
	EGameFlow GetGameModeFlow() const;

	void SetPlayerData(APlayerController* PS, FString& UniqueId);

	bool CheckEnoughPlayer();

private:
	EGameFlow GameModeFlow;

	int32 EnterPlayerNum;

	int32 MaxMatchPlayerNum;

	int32 CurMatchWaitPlayerNum;	

public:
	TArray<FPlayerData> LoginPlayerData;

	TArray<APlayerController*> LoginPlayerControllers;
};
