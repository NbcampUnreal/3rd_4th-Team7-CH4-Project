// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THMainMenuWidget.generated.h"

class UButton;

UCLASS()
class TREASUREHUNTER_API UTHMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleGameStartClicked();
	UFUNCTION()
	void HandleQuitClicked();
	
protected:
	UPROPERTY(meta = (BindWidget))
	UButton* GameStartButton;
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
};
