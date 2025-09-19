// Fill out your copyright notice in the Description page of Project Settings.


#include "THPlayerController.h"
#include "UI/THPlayerHUDWidget.h"
#include "UI/THGameOverWidget.h"
#include "THPlayerState.h"
#include "Game/THGameStateBase.h"
#include "Game/GameFlowTags.h"
#include "Item/THItemInventory.h"
#include "Item/THItemDataManager.h"
#include "Item/THItemData.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet/THAttributeSet.h"
#include "Kismet/GameplayStatics.h"


#pragma region General
void ATHPlayerController::BeginPlay()
{
	Super::BeginPlay();
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

	if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnPhaseChanged.AddDynamic(this, &ATHPlayerController::HandlePhaseChange);
	}
}

void ATHPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnPhaseChanged.RemoveDynamic(this, &ATHPlayerController::HandlePhaseChange);
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
	}
}

void ATHPlayerController::EnsureGameOver()
{
	if (!GameOverWidget)
	{
		CreateGameOverWidget();
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

		case EItemUIIndicator::FullScreenOverlay:
		{
			if (Row->UseKind == EItemUseKind::TargetDebuff && Row->VictimOverlayWidgetClass/*.IsValid()*/)
			{
				//Server_ApplyTargetOverlayToOpponent(ItemID);
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