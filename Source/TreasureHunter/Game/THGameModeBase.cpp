#include "Game/THGameModeBase.h"
#include "Game/THGameStateBase.h"
#include "Player/THPlayerState.h"
#include "Player/THTitlePlayerController.h"
#include "Player/THPlayerController.h"
#include "UI/THLoadingWidget.h"
#include "Game/GameFlowTags.h"

#include "Kismet/GameplayStatics.h"

ATHGameModeBase::ATHGameModeBase()
{
	MaxMatchPlayerNum = 2;
	MatchWaitTime = 5.0f;
	SetGameModeFlow(TAG_Game_Phase_Wait);
}

void ATHGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ATHTitlePlayerController* TitlePlayerController = Cast<ATHTitlePlayerController>(NewPlayer);
	ATHPlayerController* MatchPlayerController = Cast<ATHPlayerController>(NewPlayer);
	if (IsValid(TitlePlayerController) && IsValid(NewPlayer->Player))
	{
		EnterTitlePlayerControllers(TitlePlayerController);
	}
	else if (IsValid(MatchPlayerController) && IsValid(NewPlayer->Player))
	{
		GameStartPlayerControllers(MatchPlayerController);
	}
}

void ATHGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	ATHTitlePlayerController* ExitingPC = Cast<ATHTitlePlayerController>(Exiting);
	ATHPlayerState* ExitingPS = Cast<ATHPlayerState>(Exiting->PlayerState);
	if (IsValid(ExitingPC) && IsValid(ExitingPS))
	{
		--ServerEnterPlayerNum;

		if (GameModeFlow == TAG_Game_Phase_Wait)
		{
			if (MatchWaitPlayerControllers.Find(ExitingPC))
			{
				MatchWaitPlayerControllers.Remove(ExitingPC);
				ExitingPC->ClientCancelMatch(false);
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
		CancelPlayer->ClientCancelMatch(false);
	}

	CurMatchWaitPlayerNum = 0;
	SetGameModeFlow(TAG_Game_Phase_Match);
}

void ATHGameModeBase::LoadGame()
{
	SetGameModeFlow(TAG_Game_Phase_Loading);
}

void ATHGameModeBase::GameStart()
{
	CourseCalculate();

	for (ATHPlayerController* Player : StartPlayerControllers)
	{
		Player->SetIgnoreMoveInput(false);
		Player->SetIgnoreLookInput(false);
	}

	GetWorldTimerManager().SetTimer(
		AccumulateUpdateTimerHandle,
		this,
		&ATHGameModeBase::AccumulatePlayerDistance,
		0.1f,
		true
	);
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

void ATHGameModeBase::OpenChangeLevel(FGameplayTag NextFlow)
{
	++ReadyPlayerNum;
	TSoftObjectPtr<UWorld> LaodPath;
	if (ReadyPlayerNum == MaxMatchPlayerNum)
	{
		if (NextFlow == TAG_Game_Phase_Wait) LaodPath = MainLevelPath;
		if (NextFlow == TAG_Game_Phase_Play) LaodPath = PlayLevelPath;
	}
	else return;

	StartLevelLoad(LaodPath);
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
	//later. if we make self cancel function, I change this logic.
	if (GameModeFlow == TAG_Game_Phase_Wait)
	{
		for (ATHTitlePlayerController* CancelPlayer : MatchWaitPlayerControllers)
		{
			CancelPlayer->ClientCancelMatch(Rematch);
			--CurMatchWaitPlayerNum;
		}

		MatchWaitPlayerControllers.Empty();
	}
	else if (GameModeFlow == TAG_Game_Phase_Match)
	{
		SetGameModeFlow(TAG_Game_Phase_Wait);
		for (ATHTitlePlayerController* CancelPlayer : MatchPlayerControllers)
		{
			ATHPlayerState* CancelPS = Cast<ATHPlayerState>(CancelPlayer->PlayerState);
			CancelPlayer->Server_RequestMatchAndSetNickname_Implementation(CancelPS->Nickname);;
		}
	}	
}

void ATHGameModeBase::EnterTitlePlayerControllers(ATHTitlePlayerController* NewPlayer)
{
	++ServerEnterPlayerNum;
	LoginPlayerControllers.Add(NewPlayer);

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

void ATHGameModeBase::GameStartPlayerControllers(ATHPlayerController* Player)
{
	++StartPlayerNum;
	StartPlayerControllers.Add(Player);

	Player->DisableInput(Player);

	CheckPlayReady();
}

void ATHGameModeBase::StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad)
{
	OpenLevelPath = LevelToLoad;

	if (OpenLevelPath.IsNull()) // 경로 자체가 비어있는 경우만 체크
	{
		return;
	}

	if (!OpenLevelPath.IsValid())
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

		Streamable.RequestAsyncLoad(OpenLevelPath.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ATHGameModeBase::OnLevelLoadedReady));
	}
	else
	{
		OnLevelLoadedReady();
	}
}

void ATHGameModeBase::OnLevelLoadedReady()
{
	UWorld* LoadedWorld = OpenLevelPath.Get();
	if (LoadedWorld)
	{
		GetWorld()->ServerTravel(OpenLevelPath.ToSoftObjectPath().GetLongPackageName(), true);
	}
}

void ATHGameModeBase::CheckPlayReady()
{
	if (GameModeFlow == TAG_Game_Phase_Play) return;
	SetGameModeFlow(TAG_Game_Phase_Play);
	ReadyPlayerNum = 0;
}

void ATHGameModeBase::CourseCalculate()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Start"), FoundActors);
	if (FoundActors.Num() > 0)
	{
		StartActor = FoundActors[0];
		if (StartActor)
		{
			StartPos = StartActor->GetActorLocation();
			FoundActors.Empty();
		}
	}
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Check"), FoundActors);
	if (FoundActors.Num() > 0)
	{
		CheckActor = FoundActors[0];
		if (CheckActor)
		{
			CheckPos = CheckActor->GetActorLocation();
			FoundActors.Empty();
		}
	}
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Finish"), FoundActors);
	if (FoundActors.Num() > 0)
	{
		FinishActor = FoundActors[0];
		if (FinishActor)
		{
			FinishPos = FinishActor->GetActorLocation();
			FoundActors.Empty();
		}
	}

	if (!StartActor || !CheckActor || !FinishActor) return;

	FlatSectionDist = FVector::Dist(StartPos, CheckPos);
	ClimbSectionDist = FVector::Dist(CheckPos, FinishPos);
	TotalDist = FlatSectionDist + ClimbSectionDist;

	Section1Weight = FlatSectionDist / TotalDist;
	Section2Weight = ClimbSectionDist / TotalDist;
}

bool ATHGameModeBase::IsInFlatSection(const FVector& PlayerPos) const
{
	FVector StartToCheck = CheckPos - StartPos;
	FVector StartToPlayer = PlayerPos - StartPos;

	float Projection = FVector::DotProduct(StartToPlayer, StartToCheck.GetSafeNormal());

	return Projection < FlatSectionDist;
}

void ATHGameModeBase::AccumulatePlayerDistance()
{
	for (ATHPlayerController* Player : StartPlayerControllers)
	{
		APawn* PlayerPawn = Player->GetPawn();
		if (!PlayerPawn) continue;

		FVector PlayerPos = PlayerPawn->GetActorLocation();

		float CurrentProgress = 0.0f;
		if (IsInFlatSection(PlayerPos))
		{
			float FlatProgress = FVector::Dist(StartPos, PlayerPos) / FlatSectionDist;
			CurrentProgress = FlatProgress * Section1Weight;
		}
		else
		{
			float ClimbProgress = FVector::Dist(CheckPos, PlayerPos) / ClimbSectionDist;
			CurrentProgress = Section1Weight + ClimbProgress * Section2Weight;
		}

		uint8 QuantizedProgress = static_cast<uint8>(FMath::Clamp(CurrentProgress, 0.0f, 1.0f) * 255.0f);

		uint8 OpponentProgress = 0;
		if (StartPlayerControllers.Num() > 1)
		{
			for (ATHPlayerController* Other : StartPlayerControllers)
			{
				if (Other == Player) continue;
				APawn* OtherPawn = Other->GetPawn();
				if (!OtherPawn) continue;

				FVector OtherPos = OtherPawn->GetActorLocation();

				CurrentProgress = 0.0f;
				if (IsInFlatSection(OtherPos))
				{
					float FlatProgress = FVector::Dist(StartPos, OtherPos) / FlatSectionDist;
					CurrentProgress = FlatProgress * Section1Weight;
				}
				else
				{
					float ClimbProgress = FVector::Dist(CheckPos, OtherPos) / ClimbSectionDist;
					CurrentProgress = Section1Weight + ClimbProgress * Section2Weight;
				}

				OpponentProgress = static_cast<uint8>(FMath::Clamp(CurrentProgress, 0.0f, 1.0f) * 255.0f);
				break;
			}
		}

		Player->Client_UpdateClimb_Implementation(QuantizedProgress, OpponentProgress);
	}
}
