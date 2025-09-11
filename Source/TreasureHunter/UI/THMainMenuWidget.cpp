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
	if (EditableTextBox_Nickname)
	{
		EditableTextBox_Nickname->OnTextCommitted.AddDynamic(this, &ThisClass::OnNickNameCommitted);
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
	FString Trimmed = Nickname.TrimStartAndEnd();

	if (Trimmed.IsEmpty())
	{
		if (NicknameWarningText)
		{
			NicknameWarningText->SetText(FText::FromString(TEXT("Please Enter Your NICKNAME")));
		}
		return;
	}

	if (Trimmed.Len() < MinNicknameLength)
	{
		if (NicknameWarningText)
		{
			NicknameWarningText->SetText(FText::FromString(FString::Printf(TEXT("Nickname must be at least %d characters long."), MinNicknameLength)));
		}
		return;
	}

	if (Trimmed.Len() > MaxNicknameLength)
	{
		if (NicknameWarningText)
		{
			NicknameWarningText->SetText(FText::FromString(FString::Printf(TEXT("Nickname cannot exceed %d characters."), MaxNicknameLength)));
		}
		return;
	}

	if (NicknameWarningText)
	{
		NicknameWarningText->SetText(FText::GetEmpty());
	}

	if (auto* PC = GetOwningPlayer<ATHTitlePlayerController>())
	{
		PC->Server_RequestMatchAndSetNickname(Nickname);
	}

	if (LoadingIcon)
	{
		LoadingIcon->SetVisibility(ESlateVisibility::Visible);
		PlayAnimation(LoadingAnim);
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

void UTHMainMenuWidget::StopLoading()
{
	if (LoadingIcon && LoadingIcon->GetVisibility() == ESlateVisibility::Visible)
	{
		StopAnimation(LoadingAnim);
		LoadingIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (NicknameWarningText)
	{
		NicknameWarningText->SetText(FText::FromString(FString::Printf(TEXT("Failed to match."), MaxNicknameLength)));
	}
}
