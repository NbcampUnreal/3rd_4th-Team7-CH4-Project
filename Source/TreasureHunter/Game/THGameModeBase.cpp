#include "Game/THGameModeBase.h"
#include "Game/THGameStateBase.h"
#include "Player/THPlayerState.h"
#include "Player/THTitlePlayerController.h"
#include "Player/THPlayerController.h"
#include "UI/THLoadingWidget.h"
#include "Game/GameFlowTags.h"
#include "Game/THGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/World.h"

ATHGameModeBase::ATHGameModeBase()
{
	bUseSeamlessTravel = true;

	MaxMatchPlayerNum = 2;
	MatchWaitTime = 180.0f;
	SetGameModeFlow(TAG_Game_Phase_Wait);
}

void ATHGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("PreLogin called. Options=%s Address=%s"), *Options, *Address);
	ErrorMessage.Empty();

	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	FString NewSessionId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	const_cast<FString&>(Options).Append(FString::Printf(TEXT("?PlayerSessionId=%s"), *NewSessionId));
}

APlayerController* ATHGameModeBase::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
	UE_LOG(LogTemp, Warning, TEXT("Login called. Portal=%s Options=%s"), *Portal, *Options);
	APlayerController* PC = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	UE_LOG(LogTemp, Warning, TEXT("Login returned PC=%s Error=%s"), PC ? *PC->GetName() : TEXT("NULL"), *ErrorMessage);
	if (PC && PC->PlayerState)
	{
		ATHPlayerState* PS = Cast<ATHPlayerState>(PC->PlayerState);
		if (PS)
		{
			//PS->PlayerSessionId = PlayerSessionId;
		}
	}
	return PC;
}

void ATHGameModeBase::ApproveLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Start ApproveLogin"));
	ErrorMessage.Empty();
}

void ATHGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("PostLogin called. Player=%s"), NewPlayer ? *NewPlayer->GetName() : TEXT("NULL"));

	Super::PostLogin(NewPlayer);

	ATHTitlePlayerController* TitlePlayerController = Cast<ATHTitlePlayerController>(NewPlayer);
	if (IsValid(TitlePlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("PostLogin : %s"), *NewPlayer->GetName());
		EnterTitlePlayerControllers(TitlePlayerController);
	}

	ATHPlayerController* PlayerController = Cast<ATHPlayerController>(NewPlayer);
	if (GameModeFlow == TAG_Game_Phase_Play && IsValid(PlayerController))
	{
		//ReconnectPlayer(PlayerController);
	}

	if (ATHPlayerController* PC = Cast<ATHPlayerController>(NewPlayer))
	{
		//PC->Client_DisablePlayerControl();
	}
}

void ATHGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ATHPlayerController* MatchPlayerController = Cast<ATHPlayerController>(NewPlayer);
	if (IsValid(MatchPlayerController))
	{
		ManipluateController(MatchPlayerController, true);
		GameStartPlayerControllers(MatchPlayerController);
	}

	ATHTitlePlayerController* TitlePlayerController = Cast<ATHTitlePlayerController>(NewPlayer);
	if (IsValid(TitlePlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleStartingNewPlayer_Implementation call Enter"));
		EnterTitlePlayerControllers(TitlePlayerController);
	}
}

void ATHGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Start BeingPlay"));
	if (IsRunningDedicatedServer() && GameModeFlow == TAG_Game_Phase_Wait)
	{
		UTHGameInstance* GI = Cast<UTHGameInstance>(GetGameInstance());
		if (GI)
		{
			FString ServerAddress = TEXT("13.209.65.244");
			GI->HostSession(FName("StartSession"), 2, ServerAddress, 7777);
		}
	}

	if (GameModeFlow == TAG_Game_Phase_Play)
	{
		for (AController* PlayPC : StartPlayerControllers)
		{
			RestartPlayer(PlayPC);
		}
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
		UE_LOG(LogTemp, Warning, TEXT("Logout : %s"), *ExitingWaitPC->GetName());
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
	else if (GameModeFlow == TAG_Game_Phase_Finish)
	{
		OpenLevelPath = nullptr;
	}
}

void ATHGameModeBase::StartMatchGame(ATHTitlePlayerController* PC)
{
	if (!IsValid(PC)) return;

	UNetConnection* MatchConnection = Cast<UNetConnection>(PC->Player);
	if (!IsValid(MatchConnection)) return;

	ATHPlayerState* MatchPS = Cast<ATHPlayerState>(PC->PlayerState);
	if (!IsValid(MatchPS)) return;
	
	if (MatchWaitPlayerControllers.Contains(PC)) return;

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

	GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);
	SetGameModeFlow(TAG_Game_Phase_Match);
}

void ATHGameModeBase::LoadGame()
{
	UE_LOG(LogTemp, Error, TEXT("LoadGame"));
	SetGameModeFlow(TAG_Game_Phase_Loading);
}

void ATHGameModeBase::GameStart()
{
	CourseCalculate();

	for (ATHPlayerController* Player : StartPlayerControllers)
	{
		ACRevisionValueZ = Player->GetTargetLocation().Z - StartPos.Z;;
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

	if (OpenLevelPath == LoadPath) return;

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
	GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

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
}

void ATHGameModeBase::EnterTitlePlayerControllers(ATHTitlePlayerController* NewPlayer)
{
	++ServerEnterPlayerNum;
	LoginPlayerControllers.Add(NewPlayer);

	UNetConnection* NewConnection = NewPlayer->GetNetConnection();
	FString Address;
	if (IsValid(NewConnection))
	{
		Address = NewConnection->GetRemoteAddr()->ToString(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NewConnection is Null. Logging out."));
		//Logout(NewPlayer);
		//return;
	}

	FString GuidStr = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);;
	ATHPlayerState* NewPS = Cast<ATHPlayerState>(NewPlayer->PlayerState);
	if (NewPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("Set NewPS."));
		NewPS->PlayerAddress = Address;
		NewPS->PlayerUniqueId = GuidStr;

		SetPlayerData(Address, GuidStr);
	}
	else if (!NewPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("NewPS is Null."));
	}
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
			if (!IsValid(MatchPS)) continue;
			
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
				if (OldPS && PlayerPC)
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

	CheckPlayReady();
}

void ATHGameModeBase::ReconnectPlayer(ATHPlayerController* RePlayer)
{
}

void ATHGameModeBase::StartLevelLoad(TSoftObjectPtr<UWorld> LevelToLoad)
{
	OpenLevelPath = LevelToLoad;

	if (OpenLevelPath.IsNull()) return;

	UWorld* CurrentWorld = GetWorld();
	FString CurrentMapPath = CurrentWorld->GetOutermost()->GetName();
	FString TravelMapPath = OpenLevelPath.ToSoftObjectPath().ToString();

	if (CurrentMapPath.Equals(TravelMapPath, ESearchCase::IgnoreCase))
	{
		UE_LOG(LogTemp, Warning, TEXT("Equal Level"));
		return;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Different Level Start Travel"));
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
	if (!bIsPlayer1Ready || !bIsPlayer2Ready) return;

	SetGameModeFlow(TAG_Game_Phase_Play);
}

void ATHGameModeBase::CourseCalculate()
{
	TArray<AActor*> FoundActors;
	
	for (int32 i = 0; i < PosActorTags.Num(); i++)
	{
		TArray<AActor*> TempActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), PosActorTags[i], TempActors);
		FoundActors.Append(TempActors);
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
	// 방향 벡터
	FVector LineDir = (FinishPos - StartPos).GetSafeNormal();

	// 스타트 → 체크
	FVector StartToCheck = CheckPos - StartPos;
	float CheckProjection = FVector::DotProduct(StartToCheck, LineDir);

	// 스타트 → 플레이어
	FVector StartToPlayer = PlayerPos - StartPos;
	float PlayerProjection = FVector::DotProduct(StartToPlayer, LineDir);

	// 체크 지점 넘어감?
	if (PlayerProjection >= CheckProjection)
	{
		bPassedCheckLine = true;

		UE_LOG(LogTemp, Warning, TEXT("[IsInFlatSection] Player passed Check line. (Projection: %.2f / Check: %.2f)"),
			PlayerProjection, CheckProjection);

		return false;
	}

	return true;
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

		float OpponentTotalProgress = 0.0f;
		float ClimbOpponentProgressFloat = 0.0f;

		if (StartPlayerControllers.Num() > 1)
		{
			for (ATHPlayerController* Other : StartPlayerControllers)
			{
				if (Other == Player) continue;
				APawn* OtherPawn = Other->GetPawn();
				if (!OtherPawn) continue;

				FVector OtherPos = OtherPawn->GetActorLocation();

				float TotalProgressOther = 0.0f;
				float ClimbProgressOther = 0.0f;

				if (IsInFlatSection(OtherPos))
				{
					float FlatProgress = FVector::Dist(StartPos, OtherPos) / FlatSectionDist;
					TotalProgressOther = FlatProgress * Section1Weight;
				}
				else
				{
					ClimbProgressOther = (OtherPos.Z - CheckPos.Z - ACRevisionValueZ) / ClimbSectionDist;
					ClimbProgressOther = FMath::Clamp(ClimbProgressOther, 0.f, 1.f);
					TotalProgressOther = Section1Weight + ClimbProgressOther * Section2Weight;
				}

				OpponentTotalProgress = TotalProgressOther;
				ClimbOpponentProgressFloat = ClimbProgressOther;

				uint8 ClimbOpponentProgress = static_cast<uint8>(ClimbOpponentProgressFloat * 255.0f);
				Player->Client_UpdateClimb(ClimbQuantizedProgress, ClimbOpponentProgress);

				ATHPlayerState* PlayerState = Cast<ATHPlayerState>(Player->PlayerState);
				ATHGameStateBase* GS = Cast<ATHGameStateBase>(this->GameState);

				if (PlayerState->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_First))
				{
					bBunnyHasBeenWinning = (TotalCurrentProgress > OpponentTotalProgress);
				}
				else if (PlayerState->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_Second))
				{
					bBunnyHasBeenWinning = (TotalCurrentProgress < OpponentTotalProgress);
				}
				break;
			}
		}

		Player->Client_UpdateWinner(bBunnyHasBeenWinning);
	}
}




void ATHGameModeBase::NotifyClientLoaded(AController* ClientController)
{
	if (HasAuthority())
	{
		if (!LoadedPlayers.Contains(ClientController))
		{
			LoadedPlayers.Add(ClientController);

			TryStartGame();
		}
	}
}

void ATHGameModeBase::TryStartGame()
{
	if (LoadedPlayers.Num() >= NumExpectedPlayers)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ATHPlayerController* PC = Cast<ATHPlayerController>(*It);
			if (PC)
			{
				PC->Client_EnablePlayerControl();
			}
		}
	}
}