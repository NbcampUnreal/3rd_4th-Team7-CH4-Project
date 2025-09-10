#pragma once

#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "THTitlePlayerController.generated.h"

class UUserWidget;
class ATHGameState;

UCLASS()
class TREASUREHUNTER_API ATHTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
//private:
//
//	TObjectPtr<UTHWidgetManager> WidgetManager;
//
//	FString CustomUniqueId;

public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Server, Reliable)
	void Server_RequestMatchAndSetNickname(const FString& InNickname);

	UFUNCTION()
	void HandlePhaseChange(FGameplayTag NewPhase);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MatchmakingWidgetClass;

private:
	UPROPERTY()
	UUserWidget* ActiveWidget = nullptr;

	void ShowMainMenu();
	void ShowMatchmakingMenu();

	FDelegateHandle PhaseChangedHandle;

	////Join
	//void JoinServer(const FString& InIPAddress);

	////CustomId
	//void AssignPlayerUniqueId(FString InStr);

	//void SetCustomId(const FString& CustomId);

	//FString GetCustomId() const;
};