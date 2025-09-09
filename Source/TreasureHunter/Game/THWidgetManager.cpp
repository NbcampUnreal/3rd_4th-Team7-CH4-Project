#include "Game/THWidgetManager.h"
#include "Game/THGameModeEnum.h"
#include "Blueprint/UserWidget.h"


void UTHWidgetManager::ClientRPCDecideShowWidgetClass_Implementation(EGameFlow GameFlow, APlayerController* PC)
{
	TObjectPtr<UUserWidget> WidgetInstance = nullptr;
	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	if (GameFlow == EGameFlow::Wait)
	{
		WidgetInstance = TitleWidgetInstance;
		WidgetClass = TitleWidgetClass;
	}
	else if (GameFlow == EGameFlow::Match)
	{
		WidgetInstance = MatchWidgetInstance;
		WidgetClass = MatchWidgetClass;
	}
	else if (GameFlow == EGameFlow::Load)
	{
		WidgetInstance = LoadWidgetInstance;
		WidgetClass = LoadWidgetClass;
	}
	else if (GameFlow == EGameFlow::Play)
	{
		WidgetInstance = PlayWidgetInstance;
		WidgetClass = PlayWidgetClass;
	}
	else if (GameFlow == EGameFlow::Finish)
	{
		WidgetInstance = FinishWidgetInstance;
		WidgetClass = FinishWidgetClass;
	}
	else if (GameFlow == EGameFlow::Result)
	{
		WidgetInstance = ResultWidgetInstance;
		WidgetClass = ResultWidgetClass;
	}

	ShowGameFlowWidget(WidgetClass, WidgetInstance, PC);
}

void UTHWidgetManager::ShowGameFlowWidget(TSubclassOf<UUserWidget> UWClass, TObjectPtr<UUserWidget>& UWInstance, APlayerController* PC)
{
	UWInstance = CreateWidget<UUserWidget>(PC, UWClass);
	if (IsValid(UWInstance))
	{
		UWInstance->AddToViewport(0);

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(UWInstance->GetCachedWidget());
		PC->SetInputMode(Mode);

		PC->bShowMouseCursor = true;
	}
}
