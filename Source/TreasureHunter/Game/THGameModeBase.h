#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "THGameModeBase.generated.h"

UENUM(BlueprintType)
enum class EGameFlow : uint8
{
	Wait UMETA(DisplayName = "Wait"),
	Match UMETA(DisplayName = "Match"),
	Load UMETA(DisplayName = "Load"),
	Play UMETA(DisplayName = "Play"),
	Finish UMETA(DisplayName = "Finish"),
	Result UMETA(DisplayName = "Result")
};

struct FPlayerData
{
	FString PlayerName;
	FString PlayerUniqueId;
	APlayerState* PlayerState;
	APlayerController* PlayerController;
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

	void CreateTitleWidget(APlayerController* PC);

	bool CheckEnoughPlayer();

private:
	EGameFlow GameModeFlow;

	int32 EnterPlayerNum;

	int32 MaxMatchPlayerNum;

	int32 CurMatchWaitPlayerNum;

	//Widget Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> TitleWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> MatchWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> LoadWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> PlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> FinishWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> ResultWidgetClass;

	//Widget Instance
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> TitleWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> MatchWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> LoadWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> PlayWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> FinishWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> ResultWidgetInstance;

public:
	TArray<FPlayerData> LoginPlayerData;

	TArray<APlayerController*> LoginPlayerControllers;
};
