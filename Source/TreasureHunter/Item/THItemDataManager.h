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

    ATHItemDataManager();

protected:
    virtual void BeginPlay() override;

public:
    static ATHItemDataManager* Get(UWorld* World);

    UPROPERTY(EditDefaultsOnly, Category = "Item Data")
    UDataTable* ItemDataTable;

    UFUNCTION(BlueprintCallable)
    TSubclassOf<UGameplayAbility> GetItemAbilityClassByRow(const FName& RowName);
    
    const FTHItemData* GetItemDataByRow(const FName& RowName);

    UFUNCTION(BlueprintCallable, Category = "Item Data")
    UTexture2D* GetItemIconByRow(FName RowName);

    UFUNCTION(BlueprintPure, Category = "Item Data")
    float GetItemDurationByRow(FName RowName);

protected:
    UPROPERTY(EditAnywhere, Category = "Item Data|Perf")
    bool bPreloadAllIcons = false;

private:
    void BuildRowCache();
    void PreloadAllIcons();

    static TWeakObjectPtr<ATHItemDataManager> CachedInstance;

    TMap<FName, const FTHItemData*> RowCache;
    TMap<FName, TWeakObjectPtr<UTexture2D>> IconCache;
};