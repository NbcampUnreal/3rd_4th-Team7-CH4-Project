#pragma once

#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "THTitlePlayerController.generated.h"

class UUserWidget;

UCLASS()
class TREASUREHUNTER_API ATHTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Phase
	UFUNCTION()
	void HandlePhaseChange(FGameplayTag NewPhase);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MatchmakingWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

private:
	UPROPERTY()
	UUserWidget* ActiveWidget = nullptr;

	void ShowMainMenu();
	void ShowMatchmakingMenu();
	void ShowLoadingWidget();
	
	FGameplayTag LevelFlow;

	FDelegateHandle PhaseChangedHandle;
#pragma endregion

#pragma region Matchmaking
public:
	UFUNCTION(Client, Reliable)
	void ClientCancelMatch(bool Rematch);

	UFUNCTION(Server, Reliable)
	void Server_TrySelectSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bReady);

	UFUNCTION(Server, Reliable)
	void Server_StartMatchIfReady();
#pragma endregion
};