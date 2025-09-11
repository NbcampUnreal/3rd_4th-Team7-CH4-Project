#include "Item/THItemDataManager.h"
#include "THItemData.h"

TSubclassOf<UGameplayAbility> ATHItemDataManager::GetItemAbilityClassByID(const FString& ItemID)
{
    if (!IsValid(ItemDataTable))
    {
        UE_LOG(LogTemp, Error, TEXT("ItemDataTable is not valid in AItemDataManager."));
        return nullptr;
    }

    // FindItemDataByItemID 함수를 사용해 FItemData를 가져옵니다.
    const FTHItemData& ItemData = FindItemDataByItemID(ItemID);

    if (IsValid(ItemData.GameplayAbilityClass))
    {
        return ItemData.GameplayAbilityClass;
    }

    UE_LOG(LogTemp, Warning, TEXT("Failed to find ItemData or AbilityClass for ItemID: %s"), *ItemID);
    return nullptr;
}



const FTHItemData& ATHItemDataManager::FindItemDataByItemID(const FString& ItemIDToFind)
{
    static FTHItemData EmptyData; // 기본값 객체

    if (!IsValid(ItemDataTable))
    {
        return EmptyData;
    }

    TArray<FTHItemData*> AllRows;
    ItemDataTable->GetAllRows<FTHItemData>(TEXT("FindItemDataByItemID"), AllRows);

    for (const FTHItemData* Row : AllRows)
    {
        if (Row && Row->ItemID.Equals(ItemIDToFind, ESearchCase::IgnoreCase))
        {
            return *Row;
        }
    }

    return EmptyData;
}
