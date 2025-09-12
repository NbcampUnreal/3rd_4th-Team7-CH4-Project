#include "Item/THItemInventory.h"
#include "Item/THItemDataManager.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"	
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Abilities/GameplayAbility.h"
#include "Item/THItemData.h"
#include "UI/THPlayerHUDWidget.h"
#include "GameFramework/HUD.h"
#include "Net/UnrealNetwork.h"




UTHItemInventory::UTHItemInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTHItemInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UTHItemInventory, ItemSlot1, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UTHItemInventory, ItemSlot2, COND_OwnerOnly);
}


void UTHItemInventory::BeginPlay()
{
	Super::BeginPlay();
}


void UTHItemInventory::OnRep_ItemSlot1()
{
	OnInventorySlotChanged.Broadcast(1, ItemSlot1);
}

void UTHItemInventory::OnRep_ItemSlot2()
{
	OnInventorySlotChanged.Broadcast(2, ItemSlot2);
}



//bool UTHItemInventory::Server_AddItem_Validate(FName NewItemID)
//{
//	return true;
//}
//
//void UTHItemInventory::Server_AddItem_Implementation(FName NewItemID)
//{
//	AddItem(NewItemID);
//}



bool UTHItemInventory::AddItem(FName NewItemID)
{
	if (!GetOwner()->HasAuthority())
	{
		// 클라는 절대 AddItem 로직을 호출하지 않음
		return false;
	}

	// 서버 처리
	if (ItemSlot1.IsNone())
	{
		ItemSlot1 = NewItemID;
		return true;
	}
	else if (ItemSlot2.IsNone())
	{
		ItemSlot2 = NewItemID;
		return true;
	}
	return false;
}



void UTHItemInventory::Server_UseItem_Implementation(int32 SlotIndex)
{
	if (GetOwner()->HasAuthority())
	{
		UseItem(SlotIndex);
	}
}


void UTHItemInventory::UseItem(int32 SlotIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (UseTimeCheck)
	{
		return;
	}
	UseTimeCheck = true;
	
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
	


	if (ItemID == FName(""))
	{
		return;
	}
	ATHItemDataManager* DataManager = Cast<ATHItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass()));

	if (IsValid(DataManager))
	{
		TSubclassOf<UGameplayAbility> AbilityToActivate = DataManager->GetItemAbilityClassByRow(ItemID);

		if (AbilityToActivate)
		{
			if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetOwner()))
			{
				if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
				{
					FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityToActivate, 1, INDEX_NONE, PlayerCharacter));					
					bool bSuccess = ASC->TryActivateAbility(AbilityHandle);

					if (bSuccess)
					{
						if (SlotIndex == 2)
						{
							ItemSlot2 = FName("");
						}
						else
						{
							ItemSlot1 = FName("");
						}
					}
				}
			}
		}
	}


}



void UTHItemInventory::ResetUseTime()
{
	UseTimeCheck = false;
}