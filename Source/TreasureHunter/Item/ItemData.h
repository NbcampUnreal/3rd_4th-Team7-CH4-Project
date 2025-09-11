#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Abilities/GameplayAbility.h"

#include "ItemData.generated.h"


UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable UMETA(DisplayName = "Winner"),
    Equipment  UMETA(DisplayName = "Loser"),
    Material   UMETA(DisplayName = "Common"),
};


USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FString ItemID;
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    TSoftObjectPtr<UTexture2D> ItemIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    EItemType ItemDropType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    int32 DropWeight;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
    TSubclassOf<class UGameplayAbility> GameplayAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
    TSubclassOf<class ABaseItem> BaseItemClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	int32 CoolTime;
};

