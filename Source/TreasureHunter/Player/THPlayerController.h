// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "THPlayerController.generated.h"

class UTHPlayerHUDWidget;
class UAbilitySystemComponent;
class UTHAttributeSet;

UCLASS()
class TREASUREHUNTER_API ATHPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnRep_PlayerState() override;

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
};
