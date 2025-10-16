// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THGameOverWidget.generated.h"

class UButton;
class UImage;
class UWidgetAnimation;
class UOverlay;
class UTextBlock;

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

	UFUNCTION()
	void HandleAcceptClicked();
	UFUNCTION()
	void HandleDeclineClicked();

public:
	void SetRestartEnabled(bool bEnabled);
	void ShowDeclineText(const FText& InText);
	void ShowWaitingForOpponent();
	void ShowRematchModal();
	void HideRematchModal();

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;
	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	UImage* PlayerPreview;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ButtonAppearingAnim;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* LoadingAnim;

	UPROPERTY(meta = (BindWidget))
	UOverlay* RematchModal;

	UPROPERTY(meta = (BindWidget))
	UButton* AcceptButton;
	UPROPERTY(meta = (BindWidget))
	UButton* DeclineButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeclineText;
private:
	bool bButtonClicked = false;
};
