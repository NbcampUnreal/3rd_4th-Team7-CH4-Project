// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THMainMenuWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
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
			NicknameWarningText->SetText(FText::FromString(FString::Printf(TEXT("닉네임은 최소 %d자 이상이어야 합니다."), MinNicknameLength)));
		}
		return;
	}

	if (Trimmed.Len() > MaxNicknameLength)
	{
		if (NicknameWarningText)
		{
			NicknameWarningText->SetText(FText::FromString(FString::Printf(TEXT("닉네임은 최대 %d자까지 가능합니다."), MaxNicknameLength)));
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
