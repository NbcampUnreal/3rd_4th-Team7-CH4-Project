// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THMainMenuWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/THTitlePlayerController.h"

void UTHMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (GameStartButton)
	{
		GameStartButton->OnClicked.AddDynamic(this, &ThisClass::HandleGameStartClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &ThisClass::HandleQuitClicked);
	}

	if (LoadingIcon)
	{
		LoadingIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UTHMainMenuWidget::NativeDestruct()
{
	if (GameStartButton)
	{
		GameStartButton->OnClicked.RemoveAll(this);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UTHMainMenuWidget::HandleGameStartClicked()
{

	if (auto* PC = GetOwningPlayer<ATHTitlePlayerController>())
	{
		PC->Server_RequestMatchAndSetNickname(TEXT(""));
	}

	if (LoadingIcon)
	{
		LoadingIcon->SetVisibility(ESlateVisibility::Visible);
		PlayAnimation(LoadingAnim, 0.0f, 0, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
}

void UTHMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UTHMainMenuWidget::StopLoading()
{
	if (LoadingIcon && LoadingIcon->GetVisibility() == ESlateVisibility::Visible)
	{
		StopAnimation(LoadingAnim);
		LoadingIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (WarningText)
	{
		WarningText->SetText(FText::FromString(FString::Printf(TEXT("Failed to match."))));
	}
}
