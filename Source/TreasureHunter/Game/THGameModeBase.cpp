#include "Game/THGameModeBase.h"
#include "Game/THGameStateBase.h"
#include "Player/THPlayerState.h"
#include "Player/THTitlePlayerController.h"
#include "Player/THPlayerController.h"
#include "UI/THLoadingWidget.h"
#include "Game/GameFlowTags.h"
#include "Game/THGameInstance.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

#include "Kismet/GameplayStatics.h"

ATHGameModeBase::ATHGameModeBase()
{
	bUseSeamlessTravel = true;
	MaxMatchPlayerNum = 2;
}

void ATHGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (ATHGameStateBase* GS = GetGameState<ATHGameStateBase>())
	{
		const FGameplayTag Cur = GS->GetPhaseTag();
		if (!Cur.IsValid())
		{
			const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
			if (LevelName.Contains(TEXT("Start")))
			{
				SetGameModeFlow(TAG_Game_Phase_Wait);
			}
		}
	}
}

void ATHGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (APlayerState* PS = NewPlayer ? NewPlayer->PlayerState : nullptr)
	{
		if (ATHPlayerState* NewPS = Cast<ATHPlayerState>(PS)) ApplyOnlineNickname(NewPS);
	}

	ATHGameStateBase* GS = GetGameState<ATHGameStateBase>();
	UTHGameInstance* GI = GetGameInstance<UTHGameInstance>();

	if (HasAuthority() && GS && GetNumPlayers() == 1)
	{
		GS->HostPS = NewPlayer->PlayerState;
		GS->OnRep_HostPS();

		if (GI && GI->bIsHosting)
		{
			SetGameModeFlow(TAG_Game_Phase_Match);
			GS->TryAssignSlot(0, NewPlayer->PlayerState);
		}
	}
}


void ATHGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (ATHPlayerController* MatchPlayerController = Cast<ATHPlayerController>(NewPlayer))
	{
		if (ATHPlayerState* NewPS = Cast<ATHPlayerState>(MatchPlayerController->PlayerState)) ApplyOnlineNickname(NewPS);
		GameStartPlayerControllers(MatchPlayerController);
	}
}

void ATHGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ATHGameModeBase::SetGameModeFlow(const FGameplayTag& NewPhase)
{
	if (HasAuthority())
	{
		if (GameModeFlow == NewPhase) return; 

		GameModeFlow = NewPhase;
		if (ATHGameStateBase* GS = Cast<ATHGameStateBase>(this->GameState))
		{
			GS->SetPhase(GameModeFlow);
		}

		if (GameModeFlow == TAG_Game_Phase_Play)
		{
			GameStart();
		}
	}
}

void ATHGameModeBase::LoadGame()
{
	SetGameModeFlow(TAG_Game_Phase_Loading);
	StartLevelLoad(PlayLevelPath);
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

void ATHGameModeBase::InitialzationGameData()
{
	// 재시작 초기화 
}

void ATHGameModeBase::PlayerDetected(AActor* Player)
{
	ATHPlayerController* DetectedPC = Cast<ATHPlayerController>(Player->GetInstigatorController());
	if (IsValid(DetectedPC) && GameModeFlow != TAG_Game_Phase_Finish)
	{
		ATHGameStateBase* GS = GetGameState<ATHGameStateBase>();
		if (GS)
		{
			const bool bFirst = DetectedPC->GetPlayerState<ATHPlayerState>()->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_First);
			const FGameplayTag WinnerTag = bFirst ? TAG_Player_Character_First : TAG_Player_Character_Second;
			GS->SetWinnerTag(WinnerTag);
		}

		SetGameModeFlow(TAG_Game_Phase_Finish);
	}
}

void ATHGameModeBase::SetAfterTheGame(const FGameplayTag& AfterGameOver, ATHPlayerController* Requester)
{
	RequestRematchState = AfterGameOver;

	ATHGameStateBase* GS = Cast<ATHGameStateBase>(GameState);
	if (!IsValid(GS)) { return; }

	GS->SetRematchTag(RequestRematchState);

	ATHPlayerController* OtherPlayer = nullptr;
	for (ATHPlayerController* Other : StartPlayerControllers)
	{
		if (IsValid(Other) && Other != Requester)
		{
			OtherPlayer = Other;
			break;
		}
	}

	if (RequestRematchState == TAG_Game_Rematch_Pending)
	{
		if (!IsValid(Requester) || !IsValid(OtherPlayer) || !IsValid(Requester->PlayerState) || !IsValid(OtherPlayer->PlayerState))
		{
			UE_LOG(LogTemp, Warning, TEXT("Rematch Pending: invalid requester/responder. Treat as OpponentLeft."));
			SetAfterTheGame(TAG_Game_Rematch_OpponentLeft, nullptr);
			return;
		}

		GS->SetRematchRequester(Requester->PlayerState);
		GS->SetRematchResponder(OtherPlayer->PlayerState);

		TWeakObjectPtr<ATHGameModeBase> WeakThis(this);
		FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (!WeakThis.IsValid()) return;
				if (WeakThis->RequestRematchState != TAG_Game_Rematch_AcceptedBoth)
				{
					UE_LOG(LogTemp, Warning, TEXT("Rematch timeout"));
					WeakThis->SetAfterTheGame(TAG_Game_Rematch_Timeout, nullptr);
				}
			});

		GetWorldTimerManager().SetTimer(MatchTimerHandle, TimerDel, 10.0f, false);
		return;
	}
	else if (RequestRematchState == TAG_Game_Rematch_AcceptedBoth)
	{
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

		TWeakObjectPtr<ATHGameModeBase> WeakThis(this);
		FTimerDelegate LoadMainDel = FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (!WeakThis.IsValid()) return;
				WeakThis->SetGameModeFlow(TAG_Game_Phase_Wait);
				WeakThis->StartLevelLoad(WeakThis->MainLevelPath);
				WeakThis->RequestRematchState = FGameplayTag();
				if (ATHGameStateBase* LGS = Cast<ATHGameStateBase>(WeakThis->GameState))
				{
					LGS->ResetRematchState();
				}
			});

		GetWorldTimerManager().SetTimer(LoadMainTimerHandle, LoadMainDel, 5.0f, false);
		return;
	}

	GS->ResetRematchState();
}

void ATHGameModeBase::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);
	EnteredPlayerStates.Empty();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (APlayerState* PS = PC->PlayerState)
			{
				ActorList.Add(PS);
			}
		}
	}
}

void ATHGameModeBase::GameStartPlayerControllers(ATHPlayerController* Player)
{
	++StartPlayerNum;
	if (!StartPlayerControllers.Contains(Player))
	{
		StartPlayerControllers.Add(Player);
	}

	Player->DisableInput(Player);

	for (ATHPlayerController* PC : StartPlayerControllers)
	{
		if (ATHPlayerState* PS = Cast<ATHPlayerState>(PC->PlayerState))
		{
			if (PS->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_First))
				bIsPlayer1Ready = true;
			else if (PS->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_Second))
				bIsPlayer2Ready = true;
		}
	}

	CheckPlayReady();
}

void ATHGameModeBase::StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad)
{
	OpenLevelPath = LevelToLoad;

	if (OpenLevelPath.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("Not Found Path"));
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
	if (UWorld* LoadedWorld = OpenLevelPath.Get())
	{
		GetWorld()->ServerTravel(OpenLevelPath.ToSoftObjectPath().GetLongPackageName(), true);
	}
}

void ATHGameModeBase::CheckPlayReady()
{
	if (GameModeFlow == TAG_Game_Phase_Play) return;

	SetGameModeFlow(TAG_Game_Phase_Play);
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

void ATHGameModeBase::ApplyOnlineNickname(ATHPlayerState* PlayerState)
{
	if (!PlayerState) return;

	FString Nick = PlayerState->GetPlayerName();
	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		if (IOnlineIdentityPtr Identity = OSS->GetIdentityInterface())
		{
#if ENGINE_MAJOR_VERSION >= 5
			const FUniqueNetIdRepl& IdRepl = PlayerState->GetUniqueId();
			if (IdRepl.IsValid())
			{
				Nick = Identity->GetPlayerNickname(*IdRepl.GetUniqueNetId());
			}
#endif
		}
	}

	if (ATHPlayerState* THPS = Cast<ATHPlayerState>(PlayerState))
	{
		THPS->Nickname = Nick;
		THPS->ForceNetUpdate();
	}
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
			ClimbProgress = (PlayerPos.Z - CheckPos.Z) / ClimbSectionDist;
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
					ClimbProgress = (OtherPos.Z - CheckPos.Z) / ClimbSectionDist;
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
