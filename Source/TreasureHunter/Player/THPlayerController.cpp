// Fill out your copyright notice in the Description page of Project Settings.


#include "THPlayerController.h"
#include "UI/THPlayerHUDWidget.h"
#include "THPlayerState.h"
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

UTexture2D* ATHPlayerController::ResolveItemIcon(const FName& ItemID) const
{
	if (ATHItemDataManager* DM = Cast<ATHItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass())))
	{
		FTHItemData ItemData = DM->FindItemDataByItemID(ItemID);

		if (ItemData.ItemIcon.IsValid())
		{
			return ItemData.ItemIcon.LoadSynchronous();
		}

	}
	return nullptr;
}

#pragma endregion