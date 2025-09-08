// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THGameOverWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Player/THPlayerController.h"

void UTHGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &ThisClass::HandleRestartClicked);
	}
	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &ThisClass::HandleMainMenuClicked);
	}
}

void UTHGameOverWidget::NativeDestruct()
{
	if (RestartButton)
	{
		RestartButton->OnClicked.RemoveAll(this);
	}
	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UTHGameOverWidget::HandleRestartClicked()
{
	if (auto* PC = GetOwningPlayer<ATHPlayerController>())
	{
		// Request Server to reload the same level
		return;
	}
}

void UTHGameOverWidget::HandleMainMenuClicked()
{
	// Load Main Menu Level using soft object ptr 
}
