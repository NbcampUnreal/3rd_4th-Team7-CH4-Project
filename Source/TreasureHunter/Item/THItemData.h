#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Abilities/GameplayAbility.h"

#include "THItemData.generated.h"


UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable UMETA(DisplayName = "Winner"),
    Equipment  UMETA(DisplayName = "Loser"),
    Material   UMETA(DisplayName = "Common"),
};


USTRUCT(BlueprintType)
struct FTHItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    /*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FName ItemID =FName("");*/
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemName = FText::FromString("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    TSoftObjectPtr<UTexture2D> ItemIcon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    EItemType ItemDropType = EItemType::Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    int32 DropWeight = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
    TSubclassOf<class UGameplayAbility> GameplayAbilityClass = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
    TSubclassOf<class ATHBaseItem> BaseItemClass = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	int32 CoolTime=0;
};

