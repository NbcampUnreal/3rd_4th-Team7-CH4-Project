#include "THItemBox.h"
#include "THBaseItem.h"
#include "Item/Item_Data/THItemDataManager.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter/THPlayerCharacter.h"

#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"


ATHItemBox::ATHItemBox()
{
    bReplicates = true;
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;

    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(RootComponent);

    OverlapSphere->InitSphereRadius(100.f);
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    
}

void ATHItemBox::BeginPlay()
{
	Super::BeginPlay();
    if (OverlapSphere)
    {
        OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATHItemBox::OnOverlapBegin);
        OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ATHItemBox::OnOverlapEnd);
    }
}



void ATHItemBox::ResetUseTime()
{
    UseTimeCheck = false;
}


FName ATHItemBox::RandomItemGenerate(EItemType DropType)
{
    ATHItemDataManager* DataManager = Cast<ATHItemDataManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass()));

    if (!IsValid(DataManager) || !IsValid(DataManager->ItemDataTable))
    {
        return FName("Invalid");
    }

    TArray<FName> FilteredRowNames;
    int32 TotalWeight = 0;

    TArray<FName> RowNames = DataManager->ItemDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        const FTHItemData* ItemData = DataManager->GetItemDataByRow(RowName);
        if (!ItemData) continue;

        if ((DropType == EItemType::Equipment && ItemData->ItemDropType == EItemType::Consumable) ||
            (DropType == EItemType::Consumable && ItemData->ItemDropType == EItemType::Equipment))
        {
            continue;
        }

        FilteredRowNames.Add(RowName);
        TotalWeight += static_cast<int32>(ItemData->DropWeight);
    }

    if (FilteredRowNames.Num() == 0 || TotalWeight <= 0)
    {
        return FName("Invalid");
    }

    int32 RandomValue = FMath::RandRange(1, TotalWeight);
    int32 CurrentWeight = 0;

    for (const FName& RowName : FilteredRowNames)
    {
        const FTHItemData* ItemData = DataManager->GetItemDataByRow(RowName);
        if (!ItemData) continue;

        CurrentWeight += static_cast<int32>(ItemData->DropWeight);
        if (RandomValue <= CurrentWeight)
        {
            return RowName;
        }
    }

    return FName("Invalid");
}



void ATHItemBox::DropItem(FName RandomItemRow)
{
    if (RandomItemRow == FName("Invalid"))
    {
        return;
    }

    ATHItemDataManager* DataManager = Cast<ATHItemDataManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass()));
    if (!IsValid(DataManager))
    {
        return;
    }

    const FTHItemData* ItemData = DataManager->GetItemDataByRow(RandomItemRow);
    if (!ItemData || !IsValid(ItemData->BaseItemClass))
    {
        return;
    }

    FVector Location = GetActorLocation();
    FRotator Rotation = GetActorRotation();
    Location.Z += DropHeight;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ATHBaseItem* DroppedItem = GetWorld()->SpawnActor<ATHBaseItem>(
        ItemData->BaseItemClass, Location, Rotation, SpawnParams);

    if (DroppedItem)
    {
        DroppedItem->SetItemID(RandomItemRow);
    }
}


void ATHItemBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsLocallyControlled())
    {
        if (!InteractPromptWidget && InteractPromptClass)
        {
            InteractPromptWidget = CreateWidget<UTHInteractPromptWidget>(GetWorld(), InteractPromptClass);
            if (InteractPromptWidget)
            {
                InteractPromptWidget->AddToViewport();
            }
        }
        PlayerChar->SetInteractableActor(this);
    }
}

void ATHItemBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsLocallyControlled())
    {
        if (InteractPromptWidget)
        {
            InteractPromptWidget->RemoveFromParent();
            InteractPromptWidget = nullptr;
        }

        PlayerChar->SetInteractableActor(nullptr);
    }
}

void ATHItemBox::OpenBox()
{
    if (!HasAuthority()) return;

    if (!UseTimeCheck)
    {
        UseTimeCheck = true;
        GetWorld()->GetTimerManager().SetTimer(UseTimerHandle, this, &ATHItemBox::ResetUseTime, 0.3f, false);

        FName RandomItemID = RandomItemGenerate(EItemType::Equipment);
        DropItem(RandomItemID);

        Multicast_DestroyBox();
    }
}

void ATHItemBox::Multicast_DestroyBox_Implementation()
{
    Destroy();
}
