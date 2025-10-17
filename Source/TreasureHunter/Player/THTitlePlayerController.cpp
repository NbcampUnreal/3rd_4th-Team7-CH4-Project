#include "Player/THTitlePlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "UI/THMainMenuWidget.h"
#include "UI/THMatchmakingWidget.h"
#include "UI/THLoadingWidget.h"
#include "Game/THGameStateBase.h"
#include "THPlayerState.h"
#include "Game/THGameModeBase.h"
#include "Game/GameFlowTags.h"

void ATHTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnPhaseChanged.AddDynamic(this, &ATHTitlePlayerController::HandlePhaseChange);

		const FGameplayTag CurPhase = GS->GetPhaseTag();
		if (!CurPhase.IsValid() || CurPhase.MatchesTagExact(TAG_Game_Phase_Wait))
		{
			ShowMainMenu();
		}
		else
		{
			HandlePhaseChange(CurPhase);
		}
	}
}

void ATHTitlePlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	if (auto* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->InitializeAbilityActorInfo(aPawn);
	}
}

void ATHTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnPhaseChanged.RemoveDynamic(this, &ATHTitlePlayerController::HandlePhaseChange);
	}
	Super::EndPlay(EndPlayReason);
}

#pragma region Phase

void ATHTitlePlayerController::HandlePhaseChange(FGameplayTag NewPhase)
{
	if (!IsLocalController()) return;

	if (NewPhase.MatchesTagExact(TAG_Game_Phase_Match))
	{
		ShowMatchmakingMenu();
	}
	else if (NewPhase.MatchesTagExact(TAG_Game_Phase_Wait))
	{
		ShowMainMenu();
	}
	else if (NewPhase.MatchesTagExact(TAG_Game_Phase_Loading))
	{
		ShowLoadingWidget();
	}
}

void ATHTitlePlayerController::ShowMainMenu()
{
	if (!MainMenuWidgetClass) return;
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	ActiveWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
	if (ActiveWidget)
	{
		ActiveWidget->AddToViewport();
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void ATHTitlePlayerController::ShowMatchmakingMenu()
{
	if (!MatchmakingWidgetClass) return;
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	ActiveWidget = CreateWidget<UUserWidget>(this, MatchmakingWidgetClass);
	if (ActiveWidget)
	{
		ActiveWidget->AddToViewport();
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void ATHTitlePlayerController::ShowLoadingWidget()
{
	if (!LoadingWidgetClass) return;
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	ActiveWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
	if (ActiveWidget)
	{
		ActiveWidget->AddToViewport();
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}

	UTHLoadingWidget* LoadingWidget = Cast<UTHLoadingWidget>(ActiveWidget);
	if (LoadingWidget)
	{
		LoadingWidget->LoadProgressState();
	}
}
#pragma endregion

#pragma region Matchmaking

void ATHTitlePlayerController::BreakMatchSession()
{
	if (ATHGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->SetPhase(TAG_Game_Phase_Wait);
	}
}

void ATHTitlePlayerController::ClientCancelMatch_Implementation(bool Rematch)
{
	if (!MainMenuWidgetClass) return;

	UTHMainMenuWidget* MainMenu = Cast<UTHMainMenuWidget>(ActiveWidget);
	if (IsValid(MainMenu) && !Rematch)
	{
		MainMenu->StopLoading();
	}
}

void ATHTitlePlayerController::Server_TrySelectSlot_Implementation(int32 SlotIndex)
{
	if (ATHGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->TryAssignSlot(SlotIndex, GetPlayerState<APlayerState>());
	}
}

void ATHTitlePlayerController::Server_SetReady_Implementation(bool bReady)
{
	ATHGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr;
	ATHPlayerState* PS = GetPlayerState<ATHPlayerState>();
	if (!GS || !PS) return;

	const bool bOwned = (GS->GetSlotOwner(0) == PS) || (GS->GetSlotOwner(1) == PS);
	if (!bOwned || GS->AreSlotsLockedIn()) return;

	PS->Server_SetReady(bReady);

	if (GS->AreSlotsFilled() && GS->AreBothReady())
	{
		GS->bSlotsLockedIn = true;
		GS->OnRep_SlotsLockedIn();
		GS->ForceNetUpdate();
	}
}

void ATHTitlePlayerController::Server_StartMatchIfReady_Implementation()
{
	ATHGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr;
	ATHGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATHGameModeBase>() : nullptr;
	if (GS && GM && GS->AreSlotsLockedIn())
	{
		GM->LoadGame();
	}
}
#pragma endregion