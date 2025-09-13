// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THMatchmakingWidget.generated.h"

class UButton;
class UImage;
class ATHGameStateBase;
class ATHTitlePlayerController;

UCLASS()
class TREASUREHUNTER_API UTHMatchmakingWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* FirstPButton;
	UPROPERTY(meta = (BindWidget))
	UButton* SecondPButton;
	UPROPERTY(meta = (BindWidget))
	UButton* MatchStartButton;

	UPROPERTY(meta = (BindWidget))
	UImage* UnlockImage;
	UPROPERTY(meta = (BindWidget))
	UImage* LockImage;

private:
	TWeakObjectPtr<ATHGameStateBase> CachedGS;
	TWeakObjectPtr<ATHTitlePlayerController> CachedPC;

private:
	UFUNCTION()
	void OnFirstClicked();
	UFUNCTION()
	void OnSecondClicked();
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void RefreshUI();
};
