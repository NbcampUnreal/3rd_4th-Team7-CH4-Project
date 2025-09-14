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
	void HandleItemCooldownClient(int32 SlotIndex, float Cooltime);

	UTexture2D* ResolveItemIcon(const FName& ItemID) const;

#pragma endregion
};
