#include "Item/THItemDataManager.h"
#include "THItemData.h"

TSubclassOf<UGameplayAbility> ATHItemDataManager::GetItemAbilityClassByID(const FName& ItemID)
{
    if (!IsValid(ItemDataTable))
    {
        return nullptr;
    }

    const FTHItemData& ItemData = FindItemDataByItemID(ItemID);

    if (IsValid(ItemData.GameplayAbilityClass))
    {
        return ItemData.GameplayAbilityClass;
    }

    return nullptr;
}



FTHItemData ATHItemDataManager::FindItemDataByItemID(const FName& ItemIDToFind)
{
    static FTHItemData EmptyData;

    if (!IsValid(ItemDataTable))
    {
        return EmptyData;
    }

    TArray<FTHItemData*> AllRows;
    ItemDataTable->GetAllRows<FTHItemData>(TEXT("FindItemDataByItemID"), AllRows);

    for (const FTHItemData* Row : AllRows)
    {
        if (Row && Row->ItemID == ItemIDToFind)
        {
            return *Row;
        }
    }

    return EmptyData;
}
