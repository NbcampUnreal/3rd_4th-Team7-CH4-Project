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



UTHItemInventory::UTHItemInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTHItemInventory::BeginPlay()
{
	Super::BeginPlay();
}



bool UTHItemInventory::AddItem(FString NewItemID)
{
	if (ItemSlot1.IsEmpty())
	{
		ItemSlot1 = NewItemID;
		UE_LOG(LogTemp, Log, TEXT("Item %s added to Slot 1"), *NewItemID);
		UpdateItemImage(1, NewItemID);
		return true;
	}
	else if (ItemSlot2.IsEmpty())
	{
		ItemSlot2 = NewItemID;
		UE_LOG(LogTemp, Log, TEXT("Item %s added to Slot 2"), *NewItemID);
		UpdateItemImage(2, NewItemID);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory full. Cannot add item %s"), *NewItemID);
		return false; // 인벤토리가 가득 찼음을 알림
	}
}

void UTHItemInventory::UpdateItemImage(int32 SlotIndex, const FString& ItemID)
{
	////컨트롤러를 통해서 업데이트		
	//if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetOwner()))
	//{
	//	if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
	//	{
	//		if (AHUD* HUD = PC->GetHUD())
	//		{
	//			SetInventoryIcon(SlotIndex);

	//			if (UTHPlayerHUDWidget* HUDWidget = Cast<UTHPlayerHUDWidget>(HUD->GetUserWidgetObject()))
	//			{
	//				if (AItemDataManager* DataManager = Cast<AItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AItemDataManager::StaticClass())))
	//				{
	//					if (UItemData* ItemData = DataManager->GetItemDataByID(ItemID))
	//					{
	//						HUDWidget->SetInventoryIcon(SlotIndex, ItemData->ItemIcon);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}




void UTHItemInventory::UseItem(int32 SlotIndex)
{
	if (UseTimeCheck)
	{
		return;
	}
	UseTimeCheck = true;
	//타이머 설정
	GetWorld()->GetTimerManager().SetTimer(UseTimerHandle, this, &UTHItemInventory::ResetUseTime, 0.3f, false);


	FString ItemID;
	if (SlotIndex == 2)
	{
		ItemID = ItemSlot2;
	}
	else
	{
		ItemID = ItemSlot1;
	}
	UE_LOG(LogTemp, Log, TEXT("Attempting to use item in Slot %d: %s"), SlotIndex, *ItemID);


	if (ItemID == TEXT(""))
	{
		UE_LOG(LogTemp, Warning, TEXT("No item in Slot %d to use."), SlotIndex);
		return;
	}
	ATHItemDataManager* DataManager = Cast<ATHItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass()));

	if (IsValid(DataManager))
	{
		TSubclassOf<UGameplayAbility> AbilityToActivate = DataManager->GetItemAbilityClassByID(ItemID);

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
						UE_LOG(LogTemp, Log, TEXT("Item Ability Activated: %s"), *ItemID);
						ASC->ClearAbility(AbilityHandle);

						if (SlotIndex == 2)
						{
							ItemSlot2.Empty();
						}
						else
						{
							ItemSlot1.Empty();
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Failed to activate Item Ability: %s"), *ItemID);					
						ASC->ClearAbility(AbilityHandle);
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