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
		// 여기서 delegate 참고 구현후에 주석 없애주세요  
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
		// Client RPC 같은 거 하나 만드셔서 컨트롤러에서 HUD 업데이트 될 수 있게 함수 선언해주세요. 그래서 UseItem 성공하면 불려질 수 있도록 해주시면 끝.
		PlayerHUD->ClearInventoryIcon(SlotIndex, Cooltime);
	}
}

UTexture2D* ATHPlayerController::ResolveItemIcon(const FString& ItemID) const
{
	if (AItemDataManager* DM = Cast<AItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AItemDataManager::StaticClass())))
	{
		/* 아이템 유효성 검사 (IsValid()) 를 해야해서, 참조말고 포인터로 반환 타입 바꿔주세요!! FindItemDataByItemID 함수 
		const FItemData* Data = DM->FindItemDataByItemID(ItemID);
		if (Data && Data->ItemIcon.IsValid())
		{
			return Data->ItemIcon.LoadSynchronous();
		}*/

	}
	return nullptr;
}

#pragma endregion