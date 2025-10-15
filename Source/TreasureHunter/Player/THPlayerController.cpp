// Fill out your copyright notice in the Description page of Project Settings.


#include "THPlayerController.h"
#include "UI/THPlayerHUDWidget.h"
#include "UI/THGameOverWidget.h"
#include "THPlayerState.h"
#include "Game/THGameStateBase.h"
#include "Game/THGameModeBase.h"
#include "Game/GameFlowTags.h"
#include "Item/THItemInventory.h"
#include "Item/THItemDataManager.h"
#include "Item/THItemData.h"

#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet/THAttributeSet.h"
#include "Kismet/GameplayStatics.h"


#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Engine/LevelStreaming.h"
#include "Engine/TextureStreamingTypes.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/THGameInstance.h"

FTimerHandle ReadyPollHandle;

#pragma region General
void ATHPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() == NM_DedicatedServer || !IsLocalController())
	{
		return;
	}
	
	if (ATHGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATHGameModeBase>() : nullptr)
	{
		if (GM->GetGameModeFlow() != TAG_Game_Phase_Play) return;

		UTHGameInstance* GI = Cast<UTHGameInstance>(UGameplayStatics::GetGameInstance(this));
		if (GI)
		{
			UE_LOG(LogTemp, Warning, TEXT("Travel 시작: %s"), *GetName());
			FString ServerAddress = TEXT("13.209.65.244");
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this, ServerAddress]() {
				UTHGameInstance* GI = Cast<UTHGameInstance>(UGameplayStatics::GetGameInstance(this));
				if (GI) GI->JoinServer(ServerAddress);
				}));
		}
	}

	SetSettingForGame();
	Client_DisablePlayerControl();

	EnsureHUD();
	InitHUDBindingsFromPlayerState();

	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->InitializeAbilityActorInfo(GetPawn());
	}

	if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnPhaseChanged.AddDynamic(this, &ATHPlayerController::HandlePhaseChange);
		GS->OnRematchChanged.AddDynamic(this, &ThisClass::HandleRematchChanged);
	}
	CheckStreamingFinished();
}

void ATHPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnPhaseChanged.RemoveDynamic(this, &ATHPlayerController::HandlePhaseChange);
		GS->OnRematchChanged.RemoveDynamic(this, &ThisClass::HandleRematchChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void ATHPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	if (GetNetMode() == NM_DedicatedServer || !IsLocalController())
	{
		return;
	}

	EnsureHUD();
	InitHUDBindingsFromPlayerState();

	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->InitializeAbilityActorInfo(aPawn);

		if (HasAuthority())
		{
			PS->GiveStartupAbilities();
		}
	}

	BindInventoryDelegates(aPawn);
}

void ATHPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (GetNetMode() == NM_DedicatedServer || !IsLocalController())
	{
		return;
	}

	EnsureHUD();
	InitHUDBindingsFromPlayerState();

	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->InitializeAbilityActorInfo(GetPawn());
	}

	BindInventoryDelegates(GetPawn());
}
#pragma endregion

#pragma region Phase-Change
void ATHPlayerController::HandlePhaseChange(FGameplayTag NewPhase)
{
	if (!IsLocalController()) return;

	if (NewPhase.MatchesTagExact(TAG_Game_Phase_Finish))
	{
		if (PlayerHUD && PlayerHUD->IsInViewport())
		{
			PlayerHUD->RemoveFromParent();
		}
		EnsureGameOver();
	}
}

void ATHPlayerController::CreateGameOverWidget()
{
	if (GameOverWidget || !GameOverWidgetClass)
	{
		return;
	}
	
	GameOverWidget = CreateWidget<UTHGameOverWidget>(this, GameOverWidgetClass);
	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport();
		SetSettingModeForUI();
	}
}

void ATHPlayerController::EnsureGameOver()
{
	if (!GameOverWidget)
	{
		CreateGameOverWidget();
	}
}

void ATHPlayerController::SetSettingForGame()
{
	EnableMovement();

	FInputModeGameOnly Mode;
	Mode.SetConsumeCaptureMouseDown(false);
	SetInputMode(Mode);

	bShowMouseCursor = false;
}

void ATHPlayerController::SetSettingModeForUI()
{
	DisableMovement();

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(Mode);

	bShowMouseCursor = true;
}

void ATHPlayerController::DisableMovement()
{
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	if (APawn* P = GetPawn())
	{
		P->DisableInput(this);
		if (ACharacter* C = Cast<ACharacter>(P))
		{
			if (auto* Move = C->GetCharacterMovement())
			{
				Move->DisableMovement();
			}
		}
	}
}

void ATHPlayerController::EnableMovement()
{
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	if (APawn* P = GetPawn())
	{
		if (ACharacter* C = Cast<ACharacter>(P))
		{
			if (auto* Move = C->GetCharacterMovement())
			{
				Move->SetMovementMode(MOVE_Walking);
			}
		}
		P->EnableInput(this);
	}
}
#pragma endregion

#pragma region HUD-Open

void ATHPlayerController::CreatePlayerHUD()
{
	if (PlayerHUD || !PlayerHUDWidgetClass)
	{
		return;
	}

	PlayerHUD = CreateWidget<UTHPlayerHUDWidget>(this, PlayerHUDWidgetClass);
	if (PlayerHUD)
	{
		PlayerHUD->AddToViewport();
	}
}

void ATHPlayerController::EnsureHUD()
{
	if (!PlayerHUD)
	{
		CreatePlayerHUD();
	}
}

void ATHPlayerController::InitHUDBindingsFromPlayerState()
{
	if (!PlayerHUD || !PlayerState)
	{
		return;
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerState))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const UTHAttributeSet* Attr = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass()));
			if (Attr)
			{
				PlayerHUD->BindToAbilitySystem(ASC, Attr);
			}
		}
	}
}
#pragma endregion

#pragma region Inventory
void ATHPlayerController::BindInventoryDelegates(APawn* InPawn)
{
	if (!InPawn) return;
	if (UTHItemInventory* Inv = InPawn->FindComponentByClass<UTHItemInventory>())
	{
		Inv->OnInventorySlotChanged.AddDynamic(this, &ThisClass::HandleInventorySlotChanged);
		Inv->OnItemActivated.AddDynamic(this, &ThisClass::HandleItemActivated);
	}
}

void ATHPlayerController::HandleInventorySlotChanged(int32 SlotIndex, FName ItemID)
{
	if (!PlayerHUD) return;

	if (ItemID.IsNone())
	{
		PlayerHUD->SetInventoryIcon(SlotIndex, nullptr);
		return;
	}

	if (ATHItemDataManager* DM = ATHItemDataManager::Get(GetWorld()))
	{
		if (UTexture2D* Icon = DM->GetItemIconByRow(ItemID))
		{
			PlayerHUD->SetInventoryIcon(SlotIndex, Icon);
		}
	}
}

void ATHPlayerController::HandleItemActivated(int32 SlotIndex, FName ItemID)
{
	if (!PlayerHUD) return;

	if (ATHItemDataManager* DM = ATHItemDataManager::Get(GetWorld()))
	{
		const FTHItemData* Row = DM->GetItemDataByRow(ItemID);
		if (!Row) return;

		switch (Row->UiIndicator)
		{
		case EItemUIIndicator::InventoryProgressBarBuff:
		{
			static const FName NAME_Speed01(TEXT("Speed01"));
			static const FName NAME_Jump01(TEXT("Jump01"));

			if (ItemID == NAME_Speed01)
			{
				PlayerHUD->StartSpeedDurationBuff(Row->DurationSec);
			}
			else if (ItemID == NAME_Jump01)
			{
				PlayerHUD->StartJumpDurationBuff(Row->DurationSec);
			}
		}
		break;

		case EItemUIIndicator::TopRightInventoryBuffIcon:
		{
			if (UTexture2D* Icon = DM->GetItemIconByRow(ItemID))
			{
				PlayerHUD->ShowTopRightBuffIcon(Icon, Row->DurationSec);
			}
		}
		break;

		default:
			UE_LOG(LogTemp, Log, TEXT("[PC] Case: None/Default"));
			break;
		}
	}
}

void ATHPlayerController::Server_ApplyTargetOverlayToOpponent_Implementation(FName ItemRow)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATHPlayerController* OtherPC = Cast<ATHPlayerController>(*It);
		if (OtherPC && OtherPC != this)
		{
			OtherPC->Client_ShowTargetOverlay(ItemRow);
			break;
		}
	}
}

void ATHPlayerController::Client_ShowTargetOverlay_Implementation(FName ItemRow)
{
	if (!PlayerHUD) return;

	if (ATHItemDataManager* DM = ATHItemDataManager::Get(GetWorld()))
	{
		if (const FTHItemData* Row = DM->GetItemDataByRow(ItemRow))
		{
			if (Row->VictimOverlayWidgetClass/*.IsValid()*/)
			{
				PlayerHUD->ShowFullScreenOverlay(Row->VictimOverlayWidgetClass/*.LoadSynchronous()*/, Row->DurationSec);
			}
		}
	}
}
#pragma endregion

#pragma region Climb&Rank

void ATHPlayerController::Client_UpdateClimb_Implementation(uint8 QSelf, uint8 QOppo)
{
	const float SelfP = static_cast<float>(QSelf) / 255.f;
	const float OppoP = static_cast<float>(QOppo) / 255.f;

	if (PlayerHUD)
	{
		PlayerHUD->SetClimbUIUpdate(SelfP, OppoP);
	}
}

void ATHPlayerController::Client_UpdateWinner_Implementation(bool bBunnyWinning)
{
	if (PlayerHUD)
	{
		PlayerHUD->SetRankUIUpdate(bBunnyWinning);
	}
}
#pragma endregion

#pragma region Rematch
void ATHPlayerController::Server_RequestRematch_Implementation()
{
	if (ATHGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATHGameModeBase>() : nullptr)
	{
		// Request Rematch
		GM->SetAfterTheGame(TAG_Game_Rematch_Pending, this);
	}
}

void ATHPlayerController::Server_RespondRematch_Implementation(bool bAccept)
{
	if (ATHGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATHGameModeBase>() : nullptr)
	{
		// Rematch Response
		if (bAccept)
		{
			GM->SetAfterTheGame(TAG_Game_Rematch_AcceptedBoth, this);
		}
		else if (!bAccept)
		{
			GM->SetAfterTheGame(TAG_Game_Rematch_Declined, this);
		}
	}
}

void ATHPlayerController::Server_LeaveToMainMenu_Implementation()
{
	if (ATHGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATHGameModeBase>() : nullptr)
	{
		// Leave at GameOver
		GM->SetAfterTheGame(TAG_Game_Rematch_OpponentLeft, this);
	}
}

void ATHPlayerController::HandleRematchChanged(FGameplayTag NewTag)
{
	EnsureGameOver();
	if (!GameOverWidget) return;

	const ATHGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr;
	const bool bIAmResponder = (GS && GS->GetRematchResponder() == PlayerState);

	if (NewTag.MatchesTagExact(TAG_Game_Rematch_Pending))
	{
		GameOverWidget->SetRestartEnabled(false);
		if (bIAmResponder)  GameOverWidget->ShowRematchModal();
		else                GameOverWidget->ShowWaitingForOpponent();
	}
	else if (NewTag.MatchesTagExact(TAG_Game_Rematch_Declined))
	{
		GameOverWidget->HideRematchModal();
		GameOverWidget->SetRestartEnabled(false);
		GameOverWidget->ShowDeclineText(FText::FromString(TEXT("The other player declined rematch")));
	}
	else if (NewTag.MatchesTagExact(TAG_Game_Rematch_OpponentLeft))
	{
		GameOverWidget->HideRematchModal();
		GameOverWidget->SetRestartEnabled(false);
		GameOverWidget->ShowDeclineText(FText::FromString(TEXT("The other player has left the game")));
	}
	else if (NewTag.MatchesTagExact(TAG_Game_Rematch_Timeout))
	{
		GameOverWidget->HideRematchModal();
		GameOverWidget->SetRestartEnabled(false);
		GameOverWidget->ShowDeclineText(FText::FromString(TEXT("Rematch timed out")));
	}
	else if (NewTag.MatchesTagExact(TAG_Game_Rematch_AcceptedBoth))
	{
		GameOverWidget->HideRematchModal();
		GameOverWidget->SetRestartEnabled(false);
	}
}
#pragma endregion


void ATHPlayerController::CheckStreamingFinished()
{
	UWorld* World = GetWorld();
	if (!World) return;

	bool bStillLoading = false;
	for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
	{
		if (StreamingLevel && !StreamingLevel->IsLevelLoaded())
		{
			bStillLoading = true;
			break;
		}
	}

	if (bStillLoading)
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ATHPlayerController::CheckStreamingFinished);
		return;
	}

	
	ReadyPollHandle.Invalidate();
	const float TextureLoadSafetyDelay = 0.5f;

	World->GetTimerManager().SetTimer(ReadyPollHandle, this, &ATHPlayerController::FinishLoading, TextureLoadSafetyDelay, false);
}

void ATHPlayerController::Server_NotifyClientLoaded_Implementation()
{
	if (ATHGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATHGameModeBase>() : nullptr)
	{
		GM->NotifyClientLoaded(this);
	}
}

void ATHPlayerController::Client_DisablePlayerControl_Implementation()
{
	if (ACharacter* MyChar = GetCharacter())
		MyChar->DisableInput(this);
}

void ATHPlayerController::Client_EnablePlayerControl_Implementation()
{
	if (ACharacter* MyChar = GetCharacter())
		MyChar->EnableInput(this);
}

void ATHPlayerController::FinishLoading()
{
	if (GetNetMode() == NM_DedicatedServer || !IsLocalController())
	{
		return;
	}

	Server_NotifyClientLoaded();
}
