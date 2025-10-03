#include "Game/THGameModeBase.h"
#include "Game/THGameStateBase.h"
#include "Player/THPlayerState.h"
#include "Player/THTitlePlayerController.h"
#include "Player/THPlayerController.h"
#include "UI/THLoadingWidget.h"
#include "Game/GameFlowTags.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ATHGameModeBase::ATHGameModeBase()
{
	bUseSeamlessTravel = true;

	MaxMatchPlayerNum = 2;
	MatchWaitTime = 5.0f;
	SetGameModeFlow(TAG_Game_Phase_Wait);
}

void ATHGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ATHTitlePlayerController* TitlePlayerController = Cast<ATHTitlePlayerController>(NewPlayer);
	if (IsValid(TitlePlayerController) && IsValid(NewPlayer->Player))
	{
		EnterTitlePlayerControllers(TitlePlayerController);
	}

	ATHPlayerController* PlayerController = Cast<ATHPlayerController>(NewPlayer);
	if (GameModeFlow == TAG_Game_Phase_Play && IsValid(PlayerController) && IsValid(NewPlayer->Player))
	{
		ReconnectPlayer(PlayerController);
	}
}

void ATHGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ATHPlayerController* MatchPlayerController = Cast<ATHPlayerController>(NewPlayer);
	if (IsValid(MatchPlayerController) && IsValid(NewPlayer->Player))
	{
		ACRevisionValueZ = MatchPlayerController->GetTargetLocation().Z;
		GameStartPlayerControllers(MatchPlayerController);
	}

	ATHTitlePlayerController* TitlePlayerController = Cast<ATHTitlePlayerController>(NewPlayer);
	if (IsValid(TitlePlayerController) && IsValid(NewPlayer->Player))
	{
		EnterTitlePlayerControllers(TitlePlayerController);
	}
}

void ATHGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	ATHPlayerState* ExitingPS = Cast<ATHPlayerState>(Exiting->PlayerState);
	if (!IsValid(ExitingPS)) return;

	if (GameModeFlow == TAG_Game_Phase_Wait || GameModeFlow == TAG_Game_Phase_Match)
	{
		ATHTitlePlayerController* ExitingWaitPC = Cast<ATHTitlePlayerController>(Exiting);
		if (!IsValid(ExitingWaitPC)) return;
		
		--ServerEnterPlayerNum;

		if (GameModeFlow == TAG_Game_Phase_Wait)
		{
			if (MatchWaitPlayerControllers.Find(ExitingWaitPC))
			{
				MatchWaitPlayerControllers.Remove(ExitingWaitPC);
				ExitingWaitPC->ClientCancelMatch(false);
			}
		}
		else if (GameModeFlow == TAG_Game_Phase_Match)
		{
			if (MatchPlayerControllers.Find(ExitingWaitPC))
			{
				MatchPlayerControllers.Remove(ExitingWaitPC);
				StopMatch(true);
				UE_LOG(LogTemp, Warning, TEXT("Logout Player"));
			}
		}
	}
	else if (GameModeFlow == TAG_Game_Phase_Play)
	{
		ATHPlayerController* ExitingPlayPC = Cast<ATHPlayerController>(Exiting);
		if (!IsValid(ExitingPlayPC)) return;
		
	}
}

void ATHGameModeBase::SetGameModeFlow(const FGameplayTag& NewPhase)
{
	if (!HasAuthority()) return;
	
	GameModeFlow = NewPhase;
	
	ATHGameStateBase* GS = Cast<ATHGameStateBase>(this->GameState);
	if (!IsValid(GS)) return;

	GS->SetPhase(GameModeFlow);

	if (GameModeFlow == TAG_Game_Phase_Play)
	{
		GameStart();
	}
}

void ATHGameModeBase::StartMatchGame(ATHTitlePlayerController* PC)
{
	if (!IsValid(PC)) return;

	UNetConnection* MatchConnection = Cast<UNetConnection>(PC->Player);
	if (!IsValid(MatchConnection)) return;

	ATHPlayerState* MatchPS = Cast<ATHPlayerState>(PC->PlayerState);
	if (!IsValid(MatchPS)) return;
	
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
			FoundData->PlayerName = MatchPS->Nickname;
			MatchPS->OnRep_Nickname();

			MatchWaitPlayerControllers.Add(PC);
			if (CheckEnoughPlayer())
			{
				MatchGame();
			}
			else
			{
				StartMatchWaitTimer();
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

void ATHGameModeBase::MatchGame()
{
	for (int i = 0; i < MaxMatchPlayerNum; ++i)
	{
		ATHTitlePlayerController* MatchPlayer = MatchWaitPlayerControllers[0];
		MatchPlayerControllers.Add(MatchPlayer);
		MatchWaitPlayerControllers.Remove(MatchPlayer);
	}

	for (ATHTitlePlayerController* CancelPlayer : MatchWaitPlayerControllers)
	{
		CancelPlayer->ClientCancelMatch(false);
		UKismetSystemLibrary::QuitGame(this, CancelPlayer, EQuitPreference::Quit, false);
	}

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
		ManipluateController(Player, false);
	}

	GetWorldTimerManager().SetTimer(
		AccumulateUpdateTimerHandle,
		this,
		&ATHGameModeBase::AccumulatePlayerDistance,
		0.1f,
		true
	);
}

void ATHGameModeBase::ManipluateController(ATHPlayerController* Controller, bool Manipulate)
{
	Controller->SetIgnoreMoveInput(Manipulate);
	Controller->SetIgnoreLookInput(Manipulate);

	if (APawn* Pawn = Controller->GetPawn())
	{
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
			if (MoveComp)
			{
				if (Manipulate)
				{
					MoveComp->DisableMovement();
				}
				else
				{
					MoveComp->SetMovementMode(MOVE_Walking);
				}
			}
		}
	}
}

void ATHGameModeBase::OpenChangeLevel(FGameplayTag NextFlow)
{
	++RequestPlayerNum;
	TSoftObjectPtr<UWorld> LoadPath;
	if (RequestPlayerNum == MaxMatchPlayerNum)
	{
		if (NextFlow == TAG_Game_Phase_Play) LoadPath = PlayLevelPath;
	}
	else if (RequestRematchState == TAG_Game_Rematch_Declined ||
		RequestRematchState == TAG_Game_Rematch_OpponentLeft ||
		RequestRematchState == TAG_Game_Rematch_Timeout)
	{
		if (NextFlow == TAG_Game_Phase_Wait) LoadPath = MainLevelPath;
	}
	else return;

	RequestPlayerNum = 0;
	StartLevelLoad(LoadPath);
}

bool ATHGameModeBase::GetBunnyIsWinning() const
{
	return bBunnyHasBeenWinning;
}

void ATHGameModeBase::PlayerDetected(AActor* Player)
{
	ATHPlayerController* DetectedPC = Cast<ATHPlayerController>(Player->GetInstigatorController());
	if (IsValid(DetectedPC) && GameModeFlow != TAG_Game_Phase_Finish)
	{
		ATHGameStateBase* GS = Cast<ATHGameStateBase>(this->GameState);
		FGameplayTag WinnerTag = DetectedPC->GetPlayerState<ATHPlayerState>()->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_First) ? TAG_Player_Character_First : TAG_Player_Character_Second;
		GS->SetWinnerTag(WinnerTag);

		SetGameModeFlow(TAG_Game_Phase_Finish);
	}
}

void ATHGameModeBase::SetAfterTheGame(const FGameplayTag& AfterGameOver, ATHPlayerController* Requester)
{
	RequestRematchState = AfterGameOver;
	ATHGameStateBase* GS = Cast<ATHGameStateBase>(this->GameState);
	if (!IsValid(GS)) return;
	GS->SetRematchTag(RequestRematchState);

	ATHPlayerController* OtherPlayer = nullptr;
	for (ATHPlayerController* Other : StartPlayerControllers)
	{
		if (Other != Requester)
		{
			OtherPlayer = Other;
			break;
		}
	}

	if (RequestRematchState == TAG_Game_Rematch_Pending)
	{
		if (!IsValid(Requester) || !IsValid(OtherPlayer) || !IsValid(Requester->PlayerState) || !IsValid(OtherPlayer->PlayerState))
		{
			UE_LOG(LogTemp, Warning, TEXT("Other Player Left the Game"));
			SetAfterTheGame(TAG_Game_Rematch_OpponentLeft, nullptr);
			return;
		}

		GS->SetRematchRequester(Requester->PlayerState);
		GS->SetRematchResponder(OtherPlayer->PlayerState);

		StartMatchWaitTimer();
		return;
	}
	else if (RequestRematchState == TAG_Game_Rematch_AcceptedBoth)
	{
		//Start Rematch Game
		GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);
		ReMatchGame();
		RequestRematchState = FGameplayTag();
		GS->ResetRematchState();
		return;
	}
	else if (RequestRematchState == TAG_Game_Rematch_Declined || 
		RequestRematchState == TAG_Game_Rematch_OpponentLeft || 
		RequestRematchState == TAG_Game_Rematch_Timeout)
	{
		GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

		TWeakObjectPtr<ATHGameModeBase> WeakGM = this;
		FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([WeakGM]()
			{
				if (!WeakGM.IsValid()) return;
				WeakGM->SetGameModeFlow(TAG_Game_Phase_Wait);
				WeakGM->OpenChangeLevel(TAG_Game_Phase_Wait);
				WeakGM->RequestRematchState = FGameplayTag();
				if (ATHGameStateBase* LGS = Cast<ATHGameStateBase>(WeakGM->GameState))
				{
					LGS->ResetRematchState();
				}
			});

		//Go to MainLevel
		GetWorldTimerManager().SetTimer(
			LoadMainTimerHandle,
			TimerDel,
			5.0f,
			false
		);
		return;
	}
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
		for (ATHTitlePlayerController* TitleController : MatchWaitPlayerControllers)
		{
			if (!IsValid(TitleController)) return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

void ATHGameModeBase::StartMatchWaitTimer()
{
	FTimerDelegate TimerDel;
	if (GameModeFlow == TAG_Game_Phase_Wait)
	{
		TimerDel = FTimerDelegate::CreateLambda([this]()
			{
				StopMatch(false);
			});
	}
	else if (GameModeFlow == TAG_Game_Phase_Finish)
	{
		TWeakObjectPtr<ATHGameModeBase> WeakGM = this;
		TimerDel = FTimerDelegate::CreateLambda([WeakGM]()
			{
				if (!WeakGM.IsValid()) return;
				if (WeakGM->RequestRematchState != TAG_Game_Rematch_AcceptedBoth)
				{
					UE_LOG(LogTemp, Warning, TEXT("TimeOut"));
					WeakGM->SetAfterTheGame(TAG_Game_Rematch_Timeout, nullptr);
				}
			});
	}

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
		}

		MatchWaitPlayerControllers.Empty();
	}
	else if (GameModeFlow == TAG_Game_Phase_Match)
	{
		SetGameModeFlow(TAG_Game_Phase_Wait);
		for (ATHTitlePlayerController* CancelPlayer : MatchPlayerControllers)
		{
			ATHPlayerState* CancelPS = Cast<ATHPlayerState>(CancelPlayer->PlayerState);
			if (!IsValid(CancelPS)) continue;

			CancelPlayer->Server_RequestMatchAndSetNickname(CancelPS->Nickname);;
		}
	}

	GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);
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
	ATHPlayerState* NewPS = Cast<ATHPlayerState>(NewPlayer->PlayerState);
	NewPS->PlayerAddress = Address;
	NewPS->PlayerUniqueId = GuidStr;

	SetPlayerData(Address, GuidStr);
}

void ATHGameModeBase::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);

	if (GameModeFlow == TAG_Game_Phase_Loading)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSeamlessTravelActorList Loading"));
		for (ATHTitlePlayerController* MatchPC : MatchPlayerControllers)
		{
			ATHPlayerState* MatchPS = Cast<ATHPlayerState>(MatchPC->PlayerState);
			if (!IsValid(MatchPS)) return;
			
			EnteredPlayerStates.Add(MatchPS);
			ActorList.Add(MatchPS);
		}
	}

	if (GameModeFlow == TAG_Game_Phase_Finish)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSeamlessTravelActorList Finish"));
		for (ATHPlayerController* MainPC : StartPlayerControllers)
		{
			ATHPlayerState* MatchPS = Cast<ATHPlayerState>(MainPC->PlayerState);
			if (MatchPS)
			{
				EnteredPlayerStates.Add(MatchPS);
				ActorList.Add(MatchPS);
			}
		}
	}
}

void ATHGameModeBase::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
	if (!IsValid(C)) return;

	if (ATHPlayerController* PlayerPC = Cast<ATHPlayerController>(C))
	{		
		if (!PlayerPC->PlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState is Null"));
			for (ATHPlayerState* OldPS : EnteredPlayerStates)
			{
				if (OldPS && OldPS->GetUniqueID() == PlayerPC->PlayerState->GetUniqueID())
				{
					PlayerPC->PlayerState = OldPS;
					UE_LOG(LogTemp, Warning, TEXT("PlayerState Change!"));
					break;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState is Not Null"));
		}
	}
}

void ATHGameModeBase::GameStartPlayerControllers(ATHPlayerController* Player)
{
	if (!IsValid(Player)) return;

	if (!StartPlayerControllers.Contains(Player))
	{
		StartPlayerControllers.Add(Player);
	}

	for (ATHPlayerController* Player : StartPlayerControllers)
	{
		ATHPlayerState* NewPlayerState = Cast<ATHPlayerState>(Player->PlayerState);
		if (NewPlayerState->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_First))
		{
			bIsPlayer1Ready = true;
		}
		else if (NewPlayerState->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_Second))
		{
			bIsPlayer2Ready = true;
		}
	}

	//ManipluateController(Player, true);
	CheckPlayReady();
}

void ATHGameModeBase::ReconnectPlayer(ATHPlayerController* RePlayer)
{
}

void ATHGameModeBase::StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad)
{
	OpenLevelPath = LevelToLoad;

	if (OpenLevelPath.IsNull()) return;

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
	if (!bIsPlayer1Ready || !bIsPlayer2Ready) return;

	SetGameModeFlow(TAG_Game_Phase_Play);
}

void ATHGameModeBase::CourseCalculate()
{
	TArray<AActor*> FoundActors;
	
	for (int32 i = 0; i < PosActorTags.Num(); i++)
	{
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), PosActorTags[i], FoundActors);
	}

	for (int32 i = 0; i < FoundActors.Num(); i++)
	{
		if (!IsValid(FoundActors[i])) continue;
		
		if (FoundActors[i]->ActorHasTag(FName("Start")))
		{
			StartActor = FoundActors[i];
			StartPos = StartActor->GetActorLocation();
		}
		else if (FoundActors[i]->ActorHasTag(FName("Check")))
		{
			CheckActor = FoundActors[i];
			CheckPos = CheckActor->GetActorLocation();
		}
		else if (FoundActors[i]->ActorHasTag(FName("Finish")))
		{
			FinishActor = FoundActors[i];
			FinishPos = FinishActor->GetActorLocation();
		}
	}

	if (!IsValid(StartActor) || !IsValid(CheckActor) || !IsValid(FinishActor)) return;

	FlatSectionDist = FVector::Dist(StartPos, CheckPos);
	ClimbSectionDist = FMath::Abs(FinishPos.Z - CheckPos.Z);
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

void ATHGameModeBase::ReMatchGame()
{
	bIsPlayer1Ready = false;
	bIsPlayer2Ready = false;

	StartLevelLoad(PlayLevelPath);
}

void ATHGameModeBase::AccumulatePlayerDistance()
{
	for (ATHPlayerController* Player : StartPlayerControllers)
	{
		APawn* PlayerPawn = Player->GetPawn();
		if (!PlayerPawn) continue;

		FVector PlayerPos = PlayerPawn->GetActorLocation();

		float TotalCurrentProgress = 0.0f;
		float ClimbProgress = 0.0f;
		if (IsInFlatSection(PlayerPos))
		{
			float FlatProgress = FVector::Dist(StartPos, PlayerPos) / FlatSectionDist;
			TotalCurrentProgress = FlatProgress * Section1Weight;
		}
		else
		{
			ClimbProgress = (PlayerPos.Z - CheckPos.Z - ACRevisionValueZ) / ClimbSectionDist;
			ClimbProgress = FMath::Clamp(ClimbProgress, 0.f, 1.f);

			TotalCurrentProgress = Section1Weight + ClimbProgress * Section2Weight;
		}

		uint8 ClimbQuantizedProgress = static_cast<uint8>(FMath::Clamp(ClimbProgress, 0.0f, 1.0f) * 255.0f);
		uint8 QuantizedProgress = static_cast<uint8>(FMath::Clamp(TotalCurrentProgress, 0.0f, 1.0f) * 255.0f);

		uint8 ClimbOpponentProgress = 0;
		uint8 OpponentProgress = 0;
		if (StartPlayerControllers.Num() > 1)
		{
			for (ATHPlayerController* Other : StartPlayerControllers)
			{
				if (Other == Player) continue;
				APawn* OtherPawn = Other->GetPawn();
				if (!OtherPawn) continue;

				FVector OtherPos = OtherPawn->GetActorLocation();

				TotalCurrentProgress = 0.0f;
				if (IsInFlatSection(OtherPos))
				{
					float FlatProgress = FVector::Dist(StartPos, OtherPos) / FlatSectionDist;
					TotalCurrentProgress = FlatProgress * Section1Weight;
				}
				else
				{
					ClimbProgress = (OtherPos.Z - CheckPos.Z - ACRevisionValueZ) / ClimbSectionDist;
					ClimbProgress = FMath::Clamp(ClimbProgress, 0.f, 1.f);

					TotalCurrentProgress = Section1Weight + ClimbProgress * Section2Weight;
				}

				ClimbOpponentProgress = static_cast<uint8>(FMath::Clamp(ClimbProgress, 0.0f, 1.0f) * 255.0f);
				OpponentProgress = static_cast<uint8>(FMath::Clamp(TotalCurrentProgress, 0.0f, 1.0f) * 255.0f);
				Player->Client_UpdateClimb(ClimbQuantizedProgress, ClimbOpponentProgress);

				ATHPlayerState* PlayerState = Cast<ATHPlayerState>(Player->PlayerState);
				ATHGameStateBase* GS = Cast<ATHGameStateBase>(this->GameState);
				if (PlayerState->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_First))
				{
					if (QuantizedProgress > OpponentProgress)
					{
						bBunnyHasBeenWinning = true;
					}
					else if (QuantizedProgress < OpponentProgress)
					{
						bBunnyHasBeenWinning = false;
					}
				}
				else if (PlayerState->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_Second))
				{
					if (QuantizedProgress > OpponentProgress)
					{
						bBunnyHasBeenWinning = false;
					}
					else if(QuantizedProgress < OpponentProgress)
					{
						bBunnyHasBeenWinning = true;
					}
				}
				break;
			}
		}
		Player->Client_UpdateWinner(bBunnyHasBeenWinning);
	}
}
