#include "Item/THItemInventory.h"
#include "Item/THItemDataManager.h"
#include "Item/THItemData.h"
#include "PlayerCharacter/THPlayerCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UTHItemInventory::UTHItemInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTHItemInventory::Server_UseItem_Implementation(int32 SlotIndex)
{
	if (GetOwner()->HasAuthority())
	{
		UseItem(SlotIndex);
	}
}

void UTHItemInventory::BeginPlay()
{
	Super::BeginPlay();

	ItemDataManager = ATHItemDataManager::Get(GetWorld());
}

void UTHItemInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UTHItemInventory, ItemSlot1, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UTHItemInventory, ItemSlot2, COND_OwnerOnly);
}

void UTHItemInventory::OnRep_ItemSlot1()
{
	OnInventorySlotChanged.Broadcast(1, ItemSlot1);
}

void UTHItemInventory::OnRep_ItemSlot2()
{
	OnInventorySlotChanged.Broadcast(2, ItemSlot2);
}

FName UTHItemInventory::GetItemInSlot(int32 SlotIndex) const
{
	return (SlotIndex == 2) ? ItemSlot2 : ItemSlot1;
}

bool UTHItemInventory::AddItem(FName NewItemID)
{
	if (!GetOwner()->HasAuthority()) return false;

	if (ItemSlot1.IsNone())
	{
		ItemSlot1 = NewItemID;
		OnInventorySlotChanged.Broadcast(1, ItemSlot1);
		GetOwner()->ForceNetUpdate();
		return true;
	}
	else if (ItemSlot2.IsNone())
	{
		ItemSlot2 = NewItemID;
		OnInventorySlotChanged.Broadcast(2, ItemSlot2);
		GetOwner()->ForceNetUpdate();
		return true;
	}
	return false;
}

void UTHItemInventory::UseItem(int32 SlotIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bUseTimeCheck)
	{
		return;
	}
	bUseTimeCheck = true;
	GetWorld()->GetTimerManager().SetTimer(UseTimerHandle, this, &UTHItemInventory::ResetUseTime, 0.3f, false);


	FName ItemID;
	if (SlotIndex == 2)
	{
		ItemID = ItemSlot2;
	}
	else
	{
		ItemID = ItemSlot1;
	}

	if (ItemID.IsNone())
	{
		return;
	}

	if (!ItemDataManager)
	{
		ItemDataManager = ATHItemDataManager::Get(GetWorld());
	}

	const FTHItemData* ItemData = ItemDataManager ? ItemDataManager->GetItemDataByRow(ItemID) : nullptr;
	if (!ItemData) return;

	bool bActivated = false;
	if (ItemData->GameplayAbilityClass)
	{
		if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
			{
				const FGameplayAbilitySpec Spec(ItemData->GameplayAbilityClass, 1, INDEX_NONE, PlayerCharacter);

				//// ⭐ 아이템 데이터의 SetByCaller 데이터를 Spec에 복사합니다. ⭐
				//for (const auto& Elem : ItemData->SetByCallerData)
				//{
				//	Spec.SetByCallerTagMagnitudes.Add(Elem.Key, Elem.Value);
				//}


				const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
				bActivated = ASC->TryActivateAbility(Handle);
			}
		}
	}
	if (!bActivated) return;

	Client_NotifyItemActivated(ItemID, SlotIndex);
	if (SlotIndex == 2)
	{
		ItemSlot2 = NAME_None;
		OnInventorySlotChanged.Broadcast(2, ItemSlot2);
	}
	else
	{
		ItemSlot1 = NAME_None;
		OnInventorySlotChanged.Broadcast(1, ItemSlot1);
	}

	GetOwner()->ForceNetUpdate();
}


void UTHItemInventory::Client_NotifyItemActivated_Implementation(FName ItemRow, int32 SlotIndex)
{
	OnItemActivated.Broadcast(SlotIndex, ItemRow);
}


void UTHItemInventory::ResetUseTime()
{
	bUseTimeCheck = false;
}