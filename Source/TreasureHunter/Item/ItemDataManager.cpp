#include "Item/ItemDataManager.h"
#include "ItemData.h"

TSubclassOf<UGameplayAbility> AItemDataManager::GetItemAbilityClassByID(const FString& ItemID)
{
    if (!IsValid(ItemDataTable))
    {
        UE_LOG(LogTemp, Error, TEXT("ItemDataTable is not valid in AItemDataManager."));
        return nullptr;
    }

    // FindItemDataByItemID 함수를 사용해 FItemData를 가져옵니다.
    const FItemData& ItemData = FindItemDataByItemID(ItemID);

    if (IsValid(ItemData.GameplayAbilityClass))
    {
        return ItemData.GameplayAbilityClass;
    }

    UE_LOG(LogTemp, Warning, TEXT("Failed to find ItemData or AbilityClass for ItemID: %s"), *ItemID);
    return nullptr;
}



const FItemData& AItemDataManager::FindItemDataByItemID(const FString& ItemIDToFind)
{
    static FItemData EmptyData; // 기본값 객체

    if (!IsValid(ItemDataTable))
    {
        return EmptyData;
    }

    TArray<FItemData*> AllRows;
    ItemDataTable->GetAllRows<FItemData>(TEXT("FindItemDataByItemID"), AllRows);

    for (const FItemData* Row : AllRows)
    {
        if (Row && Row->ItemID.Equals(ItemIDToFind, ESearchCase::IgnoreCase))
        {
            return *Row;
        }
    }

    return EmptyData;
}
