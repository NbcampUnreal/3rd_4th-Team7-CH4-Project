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

    UPROPERTY(EditDefaultsOnly, Category = "Item Data")
    UDataTable* ItemDataTable;

    
    UFUNCTION(BlueprintCallable)
    TSubclassOf<UGameplayAbility> GetItemAbilityClassByID(const FName& ItemID);

    UFUNCTION(BlueprintCallable)
    FTHItemData FindItemDataByItemID(const FName& ItemIDToFind);
};