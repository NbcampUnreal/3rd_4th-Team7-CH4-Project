#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "THItemDataManager.generated.h"


struct FTHItemData;
class UGameplayAbility;

UCLASS(Blueprintable)
class TREASUREHUNTER_API ATHItemDataManager : public AActor
{
    GENERATED_BODY()

public:
    //AItemDataManager();

    UPROPERTY(EditDefaultsOnly, Category = "Item Data")
    UDataTable* ItemDataTable;

    
    UFUNCTION(BlueprintCallable)
    TSubclassOf<UGameplayAbility> GetItemAbilityClassByID(const FString& ItemID);

    UFUNCTION(BlueprintCallable)
    const FTHItemData& FindItemDataByItemID(const FString& ItemIDToFind);


    // ID를 통해 아이템 데이터를 가져오는 함수
    // const FItemData* GetItemDataByID(int32 ItemID);
};