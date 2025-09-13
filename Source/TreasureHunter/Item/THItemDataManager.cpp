#include "Item/THItemDataManager.h"
#include "THItemData.h"

//TSubclassOf<UGameplayAbility> ATHItemDataManager::GetItemAbilityClassByID(const FName& ItemID)
//{
//    if (!IsValid(ItemDataTable))
//    {
//        return nullptr;
//    }
//
//    const FTHItemData& ItemData = GetItemData(ItemID);
//
//    if (IsValid(ItemData.GameplayAbilityClass))
//    {
//        return ItemData.GameplayAbilityClass;
//    }
//
//    return nullptr;
//}
//
//
//
//FTHItemData ATHItemDataManager::FindItemDataByItemID(const FName& ItemIDToFind)
//{
//    static FTHItemData EmptyData;
//
//    if (!IsValid(ItemDataTable))
//    {
//        return EmptyData;
//    }
//
//    TArray<FTHItemData*> AllRows;
//    ItemDataTable->GetAllRows<FTHItemData>(TEXT("FindItemDataByItemID"), AllRows);
//
//    for (const FTHItemData* Row : AllRows)
//    {
//        if (Row && Row->ItemID == ItemIDToFind)
//        {
//            return *Row;
//        }
//    }
//
//    return EmptyData;
//}


TSubclassOf<UGameplayAbility> ATHItemDataManager::GetItemAbilityClassByRow(const FName& RowName)
{
    if (!IsValid(ItemDataTable))
    {
        return nullptr;
    }

    const FTHItemData* ItemData = GetItemDataByRow(RowName);
    if (ItemData && IsValid(ItemData->GameplayAbilityClass))
    {
        return ItemData->GameplayAbilityClass;
    }

    return nullptr;
}

const FTHItemData* ATHItemDataManager::GetItemDataByRow(const FName& RowName)
{
    if (!IsValid(ItemDataTable))
    {
        return nullptr;
    }

    // RowName을 ItemID로 사용
    return ItemDataTable->FindRow<FTHItemData>(RowName, TEXT("GetItemDataByRow"));
}

