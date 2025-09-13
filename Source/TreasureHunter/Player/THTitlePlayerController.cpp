#include "Player/THTitlePlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "UI/THMainMenuWidget.h"
#include "UI/THMatchmakingWidget.h"
#include "Game/THGameStateBase.h"
#include "THPlayerState.h"
#include "Game/THGameModeBase.h"
#include "Game/GameFlowTags.h"

void ATHTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		ShowMainMenu();
	}

	if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnPhaseChanged.AddDynamic(this, &ATHTitlePlayerController::HandlePhaseChange);
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
void ATHTitlePlayerController::Server_RequestMatchAndSetNickname_Implementation(const FString& InNickname)
{
	if (auto* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->Nickname = InNickname;
	}

	if (auto* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATHGameModeBase>() : nullptr)
	{
		GM->StartMatchGame(this);
	}
}

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
		FInputModeUIOnly InputMode;
		//InputMode.SetWidgetToFocus(ActiveWidget->TakeWidget());
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
		FInputModeUIOnly InputMode;
		//InputMode.SetWidgetToFocus(ActiveWidget->TakeWidget());
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}
#pragma endregion

#pragma region Matchmaking
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

	// 내가 슬롯을 점유 중일 때만 Ready 허용 & 잠금 후엔 불가
	const bool bOwned = (GS->GetSlotOwner(0) == PS) || (GS->GetSlotOwner(1) == PS);
	if (!bOwned || GS->AreSlotsLockedIn()) return;

	PS->Server_SetReady(bReady);

	// Ready 충족 시 잠금
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
		GS->SetPhase(TAG_Game_Phase_Loading);
	}
}
#pragma endregion

//void ATHTitlePlayerController::AssignPlayerUniqueId(FString InStr)
//{
//	
//}
//
//void ATHTitlePlayerController::JoinServer(const FString& InIPAddress)
//{
//	//IP
//	FName NextLevelName = FName(*InIPAddress);
//	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true);
//}
//
//void ATHTitlePlayerController::SetCustomId(const FString& CustomId)
//{
//}
//
//FString ATHTitlePlayerController::GetCustomId() const
//{
//	return FString();
//}
