#include "Game/THGameModeBase.h"
#include "Game/THGameStateBase.h"
#include "Player/THPlayerState.h"
#include "Player/THTitlePlayerController.h"
#include "Game/GameFlowTags.h"

ATHGameModeBase::ATHGameModeBase()
{
	MaxMatchPlayerNum = 2;
	MatchWaitTime = 5.0f;
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

		if (GameModeFlow == TAG_Game_Phase_Wait)
		{
			if (MatchWaitPlayerControllers.Find(ExitingPC))
			{
				MatchWaitPlayerControllers.Remove(ExitingPC);
				//ExitingPC->ClientCancelMatch(false);
			}
		}
		else if (GameModeFlow == TAG_Game_Phase_Match)
		{
			if (MatchPlayerControllers.Find(ExitingPC))
			{
				MatchPlayerControllers.Remove(ExitingPC);
				StopMatch(true);
				UE_LOG(LogTemp, Warning, TEXT("Logout Player"));
			}
		}
	}
}

void ATHGameModeBase::SetGameModeFlow(const FGameplayTag& NewPhase)
{
	if (HasAuthority())
	{
		GameModeFlow = NewPhase;
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
	if (IsValid(PC))
	{
		UNetConnection* MatchConnection = Cast<UNetConnection>(PC->Player);
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
					else
					{
						StartMatchTimer();
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
	GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

	for (int i = 0; i < MaxMatchPlayerNum; ++i)
	{
		ATHTitlePlayerController* MatchPlayer = MatchWaitPlayerControllers[0];
		MatchPlayerControllers.Add(MatchPlayer);
		MatchWaitPlayerControllers.Remove(MatchPlayer);
	}

	for (ATHTitlePlayerController* CancelPlayer : MatchWaitPlayerControllers)
	{
		//CancelPlayer->ClientCancelMatch(false);
	}

	CurMatchWaitPlayerNum = 0;
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

void ATHGameModeBase::StartMatchTimer()
{
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &ATHGameModeBase::StopMatch, false);
	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		TimerDel,
		MatchWaitTime,
		false
	);
}

void ATHGameModeBase::StopMatch(bool Rematch)
{
	//This "Wait" logic is written like this
	//because we currently don't have the function to cancel matching on self
	//later. if we make self cancel funcion, I change this logic.
	if (GameModeFlow == TAG_Game_Phase_Wait)
	{
		for (ATHTitlePlayerController* CancelPlayer : MatchWaitPlayerControllers)
		{
			//CancelPlayer->ClientCancelMatch(Rematch);
			--CurMatchWaitPlayerNum;
		}

		MatchWaitPlayerControllers.Empty();
	}
	else if (GameModeFlow == TAG_Game_Phase_Match)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cancel Match!!"));
		SetGameModeFlow(TAG_Game_Phase_Wait);
		for (ATHTitlePlayerController* CancelPlayer : MatchPlayerControllers)
		{
			ATHPlayerState* CancelPS = Cast<ATHPlayerState>(CancelPlayer->PlayerState);
			CancelPlayer->Server_RequestMatchAndSetNickname_Implementation(CancelPS->Nickname);;
		}
	}	
}
