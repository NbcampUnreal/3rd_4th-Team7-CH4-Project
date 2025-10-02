// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THMainMenuWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/THTitlePlayerController.h"
#include "Game/THGameInstance.h"

void UTHMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (GameStartButton)
	{
		GameStartButton->OnClicked.AddDynamic(this, &ThisClass::HandleGameStartClicked);
	}
	if (JoinGameButton)
	{
		JoinGameButton->OnClicked.AddDynamic(this, &ThisClass::HandleJoinGameClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &ThisClass::HandleQuitClicked);
	}

	if (LoadingIcon)
	{
		LoadingIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (auto* THGI = GetWorld() ? GetWorld()->GetGameInstance<UTHGameInstance>() : nullptr)
	{
		THGI->OnMatchmakingJoinTimeout.AddDynamic(this, &ThisClass::HandleMMJoinTimeout);
	}
}

void UTHMainMenuWidget::NativeDestruct()
{
	if (auto* THGI = GetWorld() ? GetWorld()->GetGameInstance<UTHGameInstance>() : nullptr)
	{
		THGI->OnMatchmakingJoinTimeout.RemoveDynamic(this, &ThisClass::HandleMMJoinTimeout);
	}

	if (GameStartButton)
	{
		GameStartButton->OnClicked.RemoveAll(this);
	}
	if (JoinGameButton)
	{
		JoinGameButton->OnClicked.RemoveAll(this);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UTHMainMenuWidget::HandleGameStartClicked()
{

	if (auto* THGI = GetWorld() ? GetWorld()->GetGameInstance<UTHGameInstance>() : nullptr)
		THGI->HostListen(false);

	if (LoadingIcon)
	{
		LoadingIcon->SetVisibility(ESlateVisibility::Visible);
		PlayAnimation(LoadingAnim, 0.0f, 0, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
}

void UTHMainMenuWidget::HandleJoinGameClicked()
{
	/* FOR TESTING !! */
	UGameInstance* GIraw = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UE_LOG(LogTemp, Warning, TEXT("[JoinClick] GI=%s"), GIraw ? *GIraw->GetClass()->GetName() : TEXT("nullptr"));
	/*******************/

	if (UTHGameInstance* THGI = GetWorld() ? GetWorld()->GetGameInstance<UTHGameInstance>() : nullptr)
	{
		THGI->FindAndJoin(false);
	}

	if (LoadingIcon)
	{
		LoadingIcon->SetVisibility(ESlateVisibility::Visible);
		if (LoadingAnim)
		{
			PlayAnimation(LoadingAnim, 0.f, 0, EUMGSequencePlayMode::Forward, 1.f, false);
		}
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

void UTHMainMenuWidget::HandleMMJoinTimeout()
{
	StopLoading();

	if (WarningText)
	{
		WarningText->SetText(FText::FromString(FString::Printf(TEXT("Matchmaking timed out. Please try again."))));
	}
}
