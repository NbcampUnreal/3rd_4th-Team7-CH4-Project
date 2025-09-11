#include "Game/THGameModeBase.h"
#include "Game/THGameStateBase.h"
#include "Player/THPlayerState.h"
#include "Player/THTitlePlayerController.h"
#include "Game/GameFlowTags.h"

ATHGameModeBase::ATHGameModeBase()
{
	MaxMatchPlayerNum = 2;
	SetGameModeFlow(TAG_Game_Phase_Wait);
}

void ATHGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ATHTitlePlayerController* NewPlayerController = Cast<ATHTitlePlayerController>(NewPlayer);
	if (IsValid(NewPlayerController) && IsValid(NewPlayer->Player))
	{
		++EnterPlayerNum;
		LoginPlayerControllers.Add(NewPlayerController);

		UNetConnection* NewConnection = Cast<UNetConnection>(NewPlayer->Player);
		FString Address;
		if (IsValid(NewConnection))
		{
			Address = NewConnection->GetRemoteAddr()->ToString(false);
		}
		else
		{
			Logout(NewPlayer);
			return;
		}

		FString GuidStr = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);;
		SetPlayerData(Address, GuidStr);
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

void ATHGameModeBase::SetGameModeFlow(const FGameplayTag& NewPhase)
{
	if (HasAuthority())
	{
		GameModeFlow = NewPhase;
		UE_LOG(LogTemp, Error, TEXT("Change GameFlow Phase"));
		if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
		{
			GS->SetPhase(GameModeFlow);
		}

		if (GameModeFlow == TAG_Game_Phase_Wait)
		{
			WaitGame();
		}
		else if (GameModeFlow == TAG_Game_Phase_Match)
		{
			
		}
		else if (GameModeFlow == TAG_Game_Phase_Loading)
		{
			LoadGame();
		}
		else if (GameModeFlow == TAG_Game_Phase_Play)
		{
			GameStart();
		}
		else if (GameModeFlow == TAG_Game_Phase_Finish)
		{
			FinishGame();
		}
	}
}

void ATHGameModeBase::StartMatchGame(ATHTitlePlayerController* PC)
{
	UNetConnection* MatchConnection = nullptr;

	if (IsValid(PC))
	{
		MatchConnection = Cast<UNetConnection>(PC->Player);
		if (IsValid(MatchConnection))
		{
			FString PCAddress = MatchConnection->GetRemoteAddr()->ToString(false);
			FPlayerData* FoundData = LoginPlayerData.FindByPredicate(
				[&PCAddress](const FPlayerData& Data)
				{
					return Data.PlayerAddress == PCAddress;
				});

			if (FoundData)
			{
				if (GameModeFlow == TAG_Game_Phase_Wait)
				{
					ATHPlayerState* MatchPS = Cast<ATHPlayerState>(PC->PlayerState);
					FoundData->PlayerName = MatchPS->Nickname;
					MatchPS->OnRep_Nickname();

					++CurMatchWaitPlayerNum;
					MatchWaitPlayerControllers.Add(PC);
					if (CheckEnoughPlayer())
					{
						MatchGame();
					}
				}
				else
				{
					//if players playing Game now
					UE_LOG(LogTemp, Warning, TEXT("Now Other Users Playing Game. Wait Please"));
					return;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Not Found PlayerData"));
				return;
			}
		}
	}
	else
	{
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

void ATHGameModeBase::MatchGame()
{
	for (int i = 0; i < 2; ++i)
	{
		ATHTitlePlayerController* MatchPlayer = MatchWaitPlayerControllers[i];
		MatchPlayerControllers.Add(MatchPlayer);
	}

	SetGameModeFlow(TAG_Game_Phase_Match);
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

FGameplayTag ATHGameModeBase::GetGameModeFlow() const
{
	return GameModeFlow;
}

void ATHGameModeBase::SetPlayerData(FString& Adrress, FString& UniqueId)
{
	FPlayerData NewPD;
	NewPD.PlayerAddress = Adrress;
	NewPD.PlayerUniqueId = UniqueId;

	LoginPlayerData.Add(NewPD);
}

bool ATHGameModeBase::CheckEnoughPlayer()
{
	if (MatchWaitPlayerControllers.Num() >= MaxMatchPlayerNum)
	{
		return true;
	}
	else
	{
		return false;
	}
}
