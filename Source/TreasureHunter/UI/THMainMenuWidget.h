// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "THMainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;

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
	UFUNCTION()
	void OnNickNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);

public:
	FString GetNickName() const;
	
protected:
	UPROPERTY(meta = (BindWidget))
	UButton* GameStartButton;
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_Nickname;

private:
	FString Nickname;
};
