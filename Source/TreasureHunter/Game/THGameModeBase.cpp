#include "Game/THGameModeBase.h"
#include "Player/THPlayerState.h"
#include "Player/THTitlePlayerController.h"

#include "Blueprint/UserWidget.h"

ATHGameModeBase::ATHGameModeBase()
{
	GameModeFlow = EGameFlow::Wait;
}

void ATHGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ATHTitlePlayerController* NewPlayerController = Cast<ATHTitlePlayerController>(NewPlayer);
	if (IsValid(NewPlayerController))
	{
		++EnterPlayerNum;
		LoginPlayerControllers.Add(NewPlayerController);

		FString GuidStr = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
		NewPlayerController->AssignPlayerUniqueId(GuidStr);
		SetPlayerData(NewPlayerController, GuidStr);

		if (IsValid(TitleWidgetClass))
		{
			CreateTitleWidget(NewPlayerController);
		}
	}
}

void ATHGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	ATHTitlePlayerController* ExitingPC = Cast<ATHTitlePlayerController>(Exiting);
	ATHPlayerState* ExitingPS = Cast<ATHPlayerState>(Exiting->PlayerState);
	if (IsValid(ExitingPC) && IsValid(ExitingPS))
	{
		--EnterPlayerNum;
		//FString ExitingUniqueId = ExitingPS->GetUniqueID();
	}
}

void ATHGameModeBase::SetGameModeFlow(EGameFlow GameFlow)
{
	GameModeFlow = GameFlow;
	switch (GameModeFlow)
	{
		case EGameFlow::Wait:
			WaitGame();
			break;
		case EGameFlow::Load:
			LoadGame();
			break;
		case EGameFlow::Match:
			break;
		case EGameFlow::Play:
			GameStart();
			break;
		case EGameFlow::Finish:
			FinishGame();
			break;
		case EGameFlow::Result:
			ShowResult();
			break;
	}
}

void ATHGameModeBase::StartMatchGame(ATHTitlePlayerController* PC)
{
	FString UniqueId = PC->GetCustomId();
	FPlayerData* FoundData = LoginPlayerData.FindByPredicate(
		[&UniqueId](const FPlayerData& Data)
		{
			return Data.PlayerUniqueId == UniqueId;
		});

	if (FoundData)
	{
		SetGameModeFlow(EGameFlow::Match);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Found PlayerData"));
		return;
	}
}

void ATHGameModeBase::DecidePlayCharacter()
{
}

void ATHGameModeBase::WaitGame()
{
	//Initial Game Data
}

void ATHGameModeBase::LoadGame()
{
}

void ATHGameModeBase::GameStart()
{
}

void ATHGameModeBase::FinishGame()
{
}

void ATHGameModeBase::ShowResult()
{
}

void ATHGameModeBase::InitialzationGameData()
{
}

void ATHGameModeBase::ManipluateController(bool Manipulate)
{
}

EGameFlow ATHGameModeBase::GetGameModeFlow() const
{
	return GameModeFlow;
}

void ATHGameModeBase::SetPlayerData(APlayerController* PS, FString& UniqueId)
{
	FPlayerData NewPD;
	NewPD.PlayerUniqueId = UniqueId;
	NewPD.PlayerState = PS->PlayerState;
	NewPD.PlayerController = PS;

	LoginPlayerData.Add(NewPD);
}

void ATHGameModeBase::CreateTitleWidget(APlayerController* PC)
{
	TitleWidgetInstance = CreateWidget<UUserWidget>(PC, TitleWidgetClass);
	if (IsValid(TitleWidgetInstance))
	{
		TitleWidgetInstance->AddToViewport();

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(TitleWidgetInstance->GetCachedWidget());
		PC->SetInputMode(Mode);

		PC->bShowMouseCursor = true;
	}
}

bool ATHGameModeBase::CheckEnoughPlayer()
{
	return false;
}
