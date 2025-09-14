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

#pragma region Phase
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

#pragma region HUD

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
	}
} 

void ATHPlayerController::HandleInventorySlotChanged(int32 SlotIndex, FName ItemID)
{
	if (!PlayerHUD) return;
	if (ItemID == FName("")) return;

	if (UTexture2D* Icon = ResolveItemIcon(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Icon Resolved for ItemID: %s"), *ItemID.ToString());
		PlayerHUD->SetInventoryIcon(SlotIndex, Icon);
	}
}

void ATHPlayerController::HandleItemCooldownClient(int32 SlotIndex, float Cooltime)
{
	if (PlayerHUD)
	{
		PlayerHUD->ClearInventoryIcon(SlotIndex, Cooltime);
	}
}

UTexture2D* ATHPlayerController::ResolveItemIcon(const FName& ItemRowName) const
{
	if (ATHItemDataManager* DM = Cast<ATHItemDataManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass())))
	{
		const FTHItemData* ItemData = DM->GetItemDataByRow(ItemRowName);
		if (ItemData && ItemData->ItemIcon.IsValid())
		{
			return ItemData->ItemIcon.LoadSynchronous();
		}
	}

	return nullptr;
}

#pragma endregion