#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "ItemDataManager.generated.h"


struct FItemData;
class UGameplayAbility;

UCLASS(Blueprintable)
class TREASUREHUNTER_API AItemDataManager : public AActor
{
    GENERATED_BODY()

public:
    //AItemDataManager();

    UPROPERTY(EditDefaultsOnly, Category = "Item Data")
    UDataTable* ItemDataTable;

    
    UFUNCTION(BlueprintCallable)
    TSubclassOf<UGameplayAbility> GetItemAbilityClassByID(const FString& ItemID);

    UFUNCTION(BlueprintCallable)
    const FItemData& FindItemDataByItemID(const FString& ItemIDToFind);


    // ID를 통해 아이템 데이터를 가져오는 함수
    // const FItemData* GetItemDataByID(int32 ItemID);
};