#include "Item/ItemInventory.h"
#include "Item/ItemDataManager.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"	
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Abilities/GameplayAbility.h"
#include "Item/ItemData.h"
#include "UI/THPlayerHUDWidget.h"
#include "GameFramework/HUD.h"



UItemInventory::UItemInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UItemInventory::BeginPlay()
{
	Super::BeginPlay();
}



bool UItemInventory::AddItem(FString NewItemID)
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

void UItemInventory::UpdateItemImage(int32 SlotIndex, const FString& ItemID)
{
	/*if (!ItemDataManager)
	{
		ItemDataManager = Cast<AItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AItemDataManager::StaticClass()));
		if (!ItemDataManager)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemDataManager not found in the world!"));
			return;
		}
	}

	const FItemData& ItemData = ItemDataManager->FindItemDataByItemID(ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item data not found for ItemID: %s"), *ItemID);
		return;
	}

	TSoftObjectPtr<UTexture2D> ItemIconSoftPtr = ItemData.ItemIcon;

	if (!ItemIconSoftPtr.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemIcon for %s is not valid."), *ItemID);
		return;
	}

	UTexture2D* ItemIconTexture = ItemIconSoftPtr.LoadSynchronous();
	if (!ItemIconTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load ItemIcon for %s."), *ItemID);
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
		if (PlayerController)
		{
			UTHPlayerHUDWidget* InventoryWidget = Cast<UTHPlayerHUDWidget>(PlayerController->GetHUD());
			if (InventoryWidget)
			{
				InventoryWidget->SetItemImage(SlotIndex, ItemIconTexture);
				UE_LOG(LogTemp, Log, TEXT("Successfully updated UI for slot %d with texture."), SlotIndex);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("InventoryWidget (HUD) not found."));
			}
		}
	}*/
}




void UItemInventory::UseItem(int32 SlotIndex)
{
	if (UseTimeCheck)
	{
		return;
	}
	UseTimeCheck = true;
	//타이머 설정
	GetWorld()->GetTimerManager().SetTimer(UseTimerHandle, this, &UItemInventory::ResetUseTime, 0.3f, false);


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
	AItemDataManager* DataManager = Cast<AItemDataManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AItemDataManager::StaticClass()));

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



void UItemInventory::ResetUseTime()
{
	UseTimeCheck = false;
}