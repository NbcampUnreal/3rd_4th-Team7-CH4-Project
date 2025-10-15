// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "THPlayerController.generated.h"

class UTHPlayerHUDWidget;
class UTHGameOverWidget;
class UAbilitySystemComponent;
class UTHAttributeSet;


UCLASS()
class TREASUREHUNTER_API ATHPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnRep_PlayerState() override;

#pragma region Phase
private:
	UFUNCTION()
	void HandlePhaseChange(FGameplayTag NewPhase);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTHGameOverWidget> GameOverWidgetClass;

private:
	UPROPERTY()
	UTHGameOverWidget* GameOverWidget = nullptr;

private:
	void CreateGameOverWidget();
	void EnsureGameOver();

	void SetSettingForGame();
	void SetSettingModeForUI();
	void DisableMovement();
	void EnableMovement();
#pragma endregion

#pragma region HUD
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTHPlayerHUDWidget> PlayerHUDWidgetClass;

private:
	UPROPERTY()
	UTHPlayerHUDWidget* PlayerHUD = nullptr;

private:
	void CreatePlayerHUD();
	void EnsureHUD();
	void InitHUDBindingsFromPlayerState();
#pragma endregion

#pragma region Inventory
private:
	void BindInventoryDelegates(APawn* Pawn);

	UFUNCTION()
	void HandleInventorySlotChanged(int32 SlotIndex, FName ItemID);

	UFUNCTION()
	void HandleItemActivated(int32 SlotIndex, FName ItemID);

	UFUNCTION(Server, Reliable)
	void Server_ApplyTargetOverlayToOpponent(FName ItemRow);

	UFUNCTION(Client, Reliable)
	void Client_ShowTargetOverlay(FName ItemRow);
#pragma endregion

#pragma region Climb&Rank
public:
	UFUNCTION(Client, Unreliable) // Activate when players start climbing
	void Client_UpdateClimb(uint8 QSelf, uint8 QOppo);
	UFUNCTION(Client, Reliable) // Always Active
	void Client_UpdateWinner(bool bBunnyWinning);
#pragma endregion

#pragma region Rematch
public:
	UFUNCTION(Server, Reliable)
	void Server_RequestRematch();

	UFUNCTION(Server, Reliable)
	void Server_RespondRematch(bool bAccept);

	UFUNCTION(Server, Reliable)
	void Server_LeaveToMainMenu();

private:
	UFUNCTION()
	void HandleRematchChanged(FGameplayTag NewTag);
#pragma endregion

#pragma region Loading
public:
	UFUNCTION(Server, Reliable)
	void Server_NotifyClientLoaded();

	UFUNCTION(Client, Reliable)
	void Client_DisablePlayerControl();

	UFUNCTION(Client, Reliable)
	void Client_EnablePlayerControl();

	UFUNCTION()
	void CheckStreamingFinished();

	void FinishLoading();
#pragma endregion
};
