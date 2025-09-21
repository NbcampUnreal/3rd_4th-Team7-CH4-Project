#include "THItemInventory.h"
#include "Item/Item_Data/THItemDataManager.h"
#include "Item/Item_Data/THItemData.h"
#include "Item/Item_Data/THItemRowSource.h"
#include "Player/PlayerCharacter/THPlayerCharacter.h"

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
    ItemDataManager = ATHItemDataManager::Get(GetWorld()); // 캐싱된 데이터 매니저 사용 
}

void UTHItemInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UTHItemInventory, ItemSlot1, COND_OwnerOnly); // 소유클라 UI 처리 
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
    if (!GetOwner()->HasAuthority()) return false; // 클라쪽만 처리 

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
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

    if (bUseTimeCheck) return;
    bUseTimeCheck = true;
    GetWorld()->GetTimerManager().SetTimer(UseTimerHandle, this, &UTHItemInventory::ResetUseTime, 0.3f, false);

    FName ItemID = (SlotIndex == 2) ? ItemSlot2 : ItemSlot1;
    if (ItemID.IsNone()) return;

    if (!ItemDataManager) ItemDataManager = ATHItemDataManager::Get(GetWorld());
    const FTHItemData* ItemData = ItemDataManager ? ItemDataManager->GetItemDataByRow(ItemID) : nullptr;
    if (!ItemData) return;

    bool bActivated = false;

    // RowName은 SourceObject로 보내고 Generic Ability로 한 번에 처리
    if (GenericItemAbilityClass)
    {
        if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetOwner()))
        {
            if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
            {
                UTHItemRowSource* RowSource = UTHItemRowSource::Make(this, ItemID);
                FGameplayAbilitySpec Spec(GenericItemAbilityClass, /*Level*/1, INDEX_NONE, RowSource);
                // Spec.RemoveAfterActivation = true; // Depends on Engine Version

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