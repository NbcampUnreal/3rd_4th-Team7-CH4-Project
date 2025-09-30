// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THGameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Player/THPlayerController.h"
#include "Game/GameFlowTags.h"

void UTHGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (RestartButton) RestartButton->OnClicked.AddDynamic(this, &ThisClass::HandleRestartClicked);
	if (MainMenuButton) MainMenuButton->OnClicked.AddDynamic(this, &ThisClass::HandleMainMenuClicked);

	if (ButtonAppearingAnim) PlayAnimation(ButtonAppearingAnim);

	if (DeclineText) DeclineText->SetVisibility(ESlateVisibility::Hidden);
	if (RematchModal) RematchModal->SetVisibility(ESlateVisibility::Collapsed);

	if (AcceptButton) AcceptButton->OnClicked.AddDynamic(this, &ThisClass::HandleAcceptClicked);
	if (DeclineButton) DeclineButton->OnClicked.AddDynamic(this, &ThisClass::HandleDeclineClicked);
}

void UTHGameOverWidget::NativeDestruct()
{
	if (RestartButton)  RestartButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleRestartClicked);
	if (MainMenuButton) MainMenuButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleMainMenuClicked);

	if (AcceptButton)   AcceptButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleAcceptClicked);
	if (DeclineButton)  DeclineButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleDeclineClicked);
	Super::NativeDestruct();
}

void UTHGameOverWidget::HandleRestartClicked()
{
	if (auto* PC = GetOwningPlayer<ATHPlayerController>())
	{
		PC->Server_RequestRematch();
	}
}

void UTHGameOverWidget::HandleMainMenuClicked()
{
	if (auto* PC = GetOwningPlayer<ATHPlayerController>())
	{
		PC->Server_LeaveToMainMenu();
	}
}

void UTHGameOverWidget::HandleAcceptClicked()
{
	if (auto* PC = GetOwningPlayer<ATHPlayerController>())
	{
		PC->Server_RespondRematch(true);
	}
}

void UTHGameOverWidget::HandleDeclineClicked()
{
	if (auto* PC = GetOwningPlayer<ATHPlayerController>())
	{
		PC->Server_RespondRematch(false);
	}
}

void UTHGameOverWidget::SetRestartEnabled(bool bEnabled)
{
	if (RestartButton) RestartButton->SetIsEnabled(bEnabled);
}

void UTHGameOverWidget::ShowDeclineText(const FText& InText)
{
	if (DeclineText)
	{
		DeclineText->SetText(InText);
		DeclineText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UTHGameOverWidget::ShowWaitingForOpponent()
{
	ShowDeclineText(FText::FromString(TEXT("Waiting for the other player¡¦")));
}

void UTHGameOverWidget::ShowRematchModal()
{
	if (RematchModal)
	{
		RematchModal->SetVisibility(ESlateVisibility::Visible);
		PlayAnimation(LoadingAnim, 0.0f, 0, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
}

void UTHGameOverWidget::HideRematchModal()
{
	if (RematchModal)
	{
		RematchModal->SetVisibility(ESlateVisibility::Collapsed);
		StopAnimation(LoadingAnim);
	}
}
