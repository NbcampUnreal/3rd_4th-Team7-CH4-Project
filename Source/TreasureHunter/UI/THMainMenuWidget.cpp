// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THMainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/THPlayerController.h"

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
	if (auto* PC = GetOwningPlayer<ATHPlayerController>())
	{
		// Request Match to Server
		// Go to Matchmaking UI
	}
}

void UTHMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
