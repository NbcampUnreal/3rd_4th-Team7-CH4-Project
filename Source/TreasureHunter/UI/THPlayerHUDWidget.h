// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THPlayerHUDWidget.generated.h"

class UImage;
class UProgressBar;
class UHorizontalBox;

UCLASS()
class TREASUREHUNTER_API UTHPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* CoolTimeProgressBar;

	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UHorizontalBox* SpeedBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UProgressBar* StaminaBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UProgressBar* ClimbingBar;

	UPROPERTY(meta = (BindWidget))
	UImage* SecondPlayerIMG;
	UPROPERTY(meta = (BindWidget))
	UImage* FirstPlayerIMG;
	
};
