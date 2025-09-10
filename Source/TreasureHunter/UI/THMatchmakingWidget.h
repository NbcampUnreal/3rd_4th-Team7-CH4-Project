// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THMatchmakingWidget.generated.h"

class UButton;
class UImage;

UCLASS()
class TREASUREHUNTER_API UTHMatchmakingWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

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
};
