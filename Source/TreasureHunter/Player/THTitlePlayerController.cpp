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
