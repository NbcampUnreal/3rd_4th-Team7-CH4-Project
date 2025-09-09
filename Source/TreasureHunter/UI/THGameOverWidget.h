// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THGameOverWidget.generated.h"

class UButton;
class UWorld;
/**
 * 
 */
UCLASS()
class TREASUREHUNTER_API UTHGameOverWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleRestartClicked();
	UFUNCTION()
	void HandleMainMenuClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;
	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton;
};
