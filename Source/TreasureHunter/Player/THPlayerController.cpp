// Fill out your copyright notice in the Description page of Project Settings.


#include "THPlayerController.h"
#include "UI/THPlayerHUDWidget.h"
#include "THPlayerState.h"
#include "Item/ItemInventory.h"
#include "Item/ItemDataManager.h"
#include "Item/ItemData.h"

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

void ATHPlayerController::BindInventoryDelegates(APawn* Pawn)
{
	if (!Pawn) return;
	if (UItemInventory* Inv = Pawn->FindComponentByClass<UItemInventory>())
	{
		// ���⼭ delegate ���� �����Ŀ� �ּ� �����ּ���  
		//Inv->OnInventorySlotChanged.AddUObject(this, &ThisClass::HandleInventorySlotChanged);
	}
} 

void ATHPlayerController::HandleInventorySlotChanged(int32 SlotIndex, const FString& ItemID)
{
	if (!PlayerHUD) return;
	if (ItemID.IsEmpty())
	{
		return;
	}

	if (UTexture2D* Icon = ResolveItemIcon(ItemID))
	{
		PlayerHUD->SetInventoryIcon(SlotIndex, Icon);
	}
}

void ATHPlayerController::HandleItemCooldownClient(int32 SlotIndex, float Cooltime)
{
	if (PlayerHUD)
	{
		// Client RPC ���� �� �ϳ� ����ż� ��Ʈ�ѷ����� HUD ������Ʈ �� �� �ְ� �Լ� �������ּ���. �׷��� UseItem �����ϸ� �ҷ��� �� �ֵ��� ���ֽø� ��.
		PlayerHUD->ClearInventoryIcon(SlotIndex, Cooltime);
	}
}

UTexture2D* ATHPlayerController::ResolveItemIcon(const FString& ItemID) const
{
	if (AItemDataManager* DM = Cast<AItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AItemDataManager::StaticClass())))
	{
		/* ������ ��ȿ�� �˻� (IsValid()) �� �ؾ��ؼ�, �������� �����ͷ� ��ȯ Ÿ�� �ٲ��ּ���!! FindItemDataByItemID �Լ� 
		const FItemData* Data = DM->FindItemDataByItemID(ItemID);
		if (Data && Data->ItemIcon.IsValid())
		{
			return Data->ItemIcon.LoadSynchronous();
		}*/

	}
	return nullptr;
}

#pragma endregion