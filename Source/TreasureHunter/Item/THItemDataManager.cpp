#include "Item/THItemDataManager.h"
#include "THItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Player/THPlayerController.h"
#include "Player/THPlayerState.h"
#include "Game/GameFlowTags.h"
#include "AbilitySystemComponent.h"
#include "Game/THGameModeBase.h"

TWeakObjectPtr<ATHItemDataManager> ATHItemDataManager::CachedInstance;

ATHItemDataManager::ATHItemDataManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

void ATHItemDataManager::BeginPlay()
{
    Super::BeginPlay();
    if (!CachedInstance.IsValid())
    {
        CachedInstance = this;
    }

    BuildRowCache();

    if (bPreloadAllIcons)
    {
        PreloadAllIcons();
    }
}

#pragma region Cache 
ATHItemDataManager* ATHItemDataManager::Get(UWorld* World)
{
    if (CachedInstance.IsValid())
    {
        return CachedInstance.Get();
    }
    if (!World) return nullptr;

    if (AActor* Found = UGameplayStatics::GetActorOfClass(World, StaticClass()))
    {
        CachedInstance = Cast<ATHItemDataManager>(Found);
    }
    return CachedInstance.Get();
}

void ATHItemDataManager::BuildRowCache()
{
    RowCache.Empty();

    if (!ItemDataTable) return;

    static const FString Ctx(TEXT("BuildRowCache"));
    TArray<FName> RowNames = ItemDataTable->GetRowNames();
    RowCache.Reserve(RowNames.Num());

    for (const FName& Row : RowNames)
    {
        if (const FTHItemData* Data = ItemDataTable->FindRow<FTHItemData>(Row, Ctx))
        {
            RowCache.Add(Row, Data);
        }
    }
}

void ATHItemDataManager::PreloadAllIcons()
{
    if (!ItemDataTable) return;

    for (const TPair<FName, const FTHItemData*>& Kvp : RowCache)
    {
        const FName Row = Kvp.Key;
        const FTHItemData* Data = Kvp.Value;
        if (!Data) continue;

        if (Data->ItemIcon.IsValid())
        {
            IconCache.Add(Row, Data->ItemIcon.Get());
            continue;
        }

        if (Data->ItemIcon.ToSoftObjectPath().IsValid())
        {
            if (UTexture2D* Loaded = Data->ItemIcon.LoadSynchronous())
            {
                IconCache.Add(Row, Loaded);
            }
        }
    }
}
#pragma endregion

#pragma region GetItem
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

    if (const FTHItemData** Found = RowCache.Find(RowName))
    {
        return *Found;
    }

    static const FString Ctx(TEXT("GetItemDataByRow"));
    if (const FTHItemData* Data = ItemDataTable->FindRow<FTHItemData>(RowName, Ctx))
    {
        RowCache.Add(RowName, Data);
        return Data;
    }
    return nullptr;
}

UTexture2D* ATHItemDataManager::GetItemIconByRow(FName RowName)
{
    if (TWeakObjectPtr<UTexture2D>* Cached = IconCache.Find(RowName))
    {
        return Cached->Get();
    }

    if (const FTHItemData* Data = GetItemDataByRow(RowName))
    {
        if (Data->ItemIcon.IsValid())
        {
            UTexture2D* Tex = Data->ItemIcon.Get();
            IconCache.Add(RowName, Tex);
            return Tex;
        }
        if (Data->ItemIcon.ToSoftObjectPath().IsValid())
        {
            UTexture2D* Loaded = Data->ItemIcon.LoadSynchronous();
            IconCache.Add(RowName, Loaded);
            return Loaded;
        }
    }
    return nullptr;
}

float ATHItemDataManager::GetItemDurationByRow(FName RowName)
{
    if (const FTHItemData* Data = GetItemDataByRow(RowName))
    {
        return Data->DurationSec;
    }
    return 0.f;
}

#pragma endregion



bool ATHItemDataManager::WhoWinner(ATHPlayerController* PC)
{
    if (!PC) return false;
 
	ATHPlayerState* PS = Cast<ATHPlayerState>(PC->PlayerState);
	if (!PS) return false;

    bool bIsBunny = PS->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Player_Character_First);


    ATHGameModeBase* GameModeA = Cast<ATHGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
    
    bool BunnyWinner = GameModeA->GetBunnyIsWinning();

    if (bIsBunny == BunnyWinner)
    {
		return true;
    }


    return false;

}