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

UENUM(BlueprintType)
enum class EItemUIIndicator : uint8
{
    None, // No HUD Update
    InventoryProgressBarBuff, // BottomLeft Stat Bar
    TopRightInventoryBuffIcon, // Top Right Buff Icon
    FullScreenOverlay // Full Screen Overlay Effect
};

UENUM(BlueprintType)
enum class EItemUseKind : uint8
{
    Instant, // No Duration
    DurationBuff,
    Deployable, // Can be installed in world
    TargetDebuff,
};

USTRUCT(BlueprintType)
struct FTHItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    /*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FName ItemID =FName("");*/
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Data")
	FText ItemName = FText::FromString("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Display")
    TSoftObjectPtr<UTexture2D> ItemIcon = nullptr;



    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Drop")
    EItemType ItemDropType = EItemType::Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Drop")
    int32 DropWeight = 0;



    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|UI")
    EItemUIIndicator UiIndicator = EItemUIIndicator::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|UI")
    EItemUseKind UseKind = EItemUseKind::Instant;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    float DurationSec = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    TSoftClassPtr<UUserWidget> VictimOverlayWidgetClass; // If FullScreenOverlay



    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Gameplay")
    TSubclassOf<class UGameplayAbility> GameplayAbilityClass = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
    TSubclassOf<class ATHBaseItem> BaseItemClass = nullptr;
};

