// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THMainMenuWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
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
	if (EditableTextBox_Nickname)
	{
		EditableTextBox_Nickname->OnTextCommitted.AddDynamic(this, &ThisClass::OnNickNameCommitted);
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
	if (Nickname.TrimStartAndEnd().IsEmpty()) return;

	if (auto* PC = GetOwningPlayer<ATHTitlePlayerController>())
	{
		PC->Server_RequestMatchAndSetNickname(Nickname);
	}
}

void UTHMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UTHMainMenuWidget::OnNickNameCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnUserMovedFocus)
	{
		Nickname = Text.ToString();
	}
}

FString UTHMainMenuWidget::GetNickName() const
{
	return Nickname;
}
