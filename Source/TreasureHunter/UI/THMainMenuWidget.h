// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THMainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UImage;
class UWidgetAnimation;

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
	void HandleJoinGameClicked();
	UFUNCTION()
	void HandleQuitClicked();

public:
	void StopLoading();
	
protected:
	UPROPERTY(meta = (BindWidget))
	UButton* GameStartButton;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinGameButton;
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarningText;


	UPROPERTY(meta = (BindWidget))
	UImage* LoadingIcon;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* LoadingAnim;

};
