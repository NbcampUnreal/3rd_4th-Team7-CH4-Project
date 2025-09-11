#include "Item/THItemBox.h"
#include "THItemData.h"
#include "Item/THItemDataManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Item/THBaseItem.h"
#include "PlayerCharacter/THPlayerCharacter.h"


ATHItemBox::ATHItemBox()
{
	PrimaryActorTick.bCanEverTick = true;
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

void ATHItemBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ATHItemBox::ResetUseTime()
{
    UseTimeCheck = false;
}


FName ATHItemBox::RandomItemGenerate(EItemType DropType)
{
    ATHItemDataManager* DataManager = Cast<ATHItemDataManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass()));

    if (!IsValid(DataManager))
    {
        return FName("Invalid");
    }

    TArray<const FTHItemData*> FilteredItems;
    int32 TotalWeight = 0;

    TArray<FTHItemData*> AllItemData;

    if (!IsValid(DataManager->ItemDataTable))
    {
        return TEXT("Invalid");
	}

    DataManager->ItemDataTable->GetAllRows(TEXT(""), AllItemData);

    for (const FTHItemData* ItemData : AllItemData)
    {
        if ((DropType == EItemType::Equipment && ItemData->ItemDropType == EItemType::Consumable) ||
            (DropType == EItemType::Consumable && ItemData->ItemDropType == EItemType::Equipment))
        {
            continue;
        }

        FilteredItems.Add(ItemData);
        TotalWeight += static_cast<int32>(ItemData->DropWeight);
    }

    if (FilteredItems.Num() == 0 || TotalWeight <= 0)
    {
        return TEXT("Invalid");
    }

    int32 RandomValue = FMath::RandRange(1, TotalWeight);
    int32 CurrentWeight = 0;

    for (const FTHItemData* SelectedItem : FilteredItems)
    {
        CurrentWeight += static_cast<int32>(SelectedItem->DropWeight);
        if (RandomValue <= CurrentWeight)
        {
            return SelectedItem->ItemID;
        }
    }

    return TEXT("Invalid");
}


void ATHItemBox::DropItem(FName RandomItemID)
{	
    if (RandomItemID == FName("Invalid"))
    {
        return;
    }

    ATHItemDataManager* DataManager = Cast<ATHItemDataManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass()));
	if (!IsValid(DataManager))
	{
		return;
	}

    const FTHItemData* ItemData = nullptr;
    TArray<FTHItemData*> AllRows;
    DataManager->ItemDataTable->GetAllRows(TEXT(""), AllRows);

    for (const FTHItemData* Row : AllRows)
    {
        if (Row && Row->ItemID == RandomItemID)
        {
            ItemData = Row;
            break;
        }
    }

    if (!ItemData || !IsValid(ItemData->BaseItemClass))
    {
        return;
    }

    FVector Location = GetActorLocation();
    FRotator Rotation = GetActorRotation();

	Location.Z += DropHeight;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ATHBaseItem* DroppedItem = GetWorld()->SpawnActor<ATHBaseItem>(
        ItemData->BaseItemClass, Location, Rotation, SpawnParams);

    if (DroppedItem)
    {
        DroppedItem->SetItemID(RandomItemID);
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
