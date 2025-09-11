#include "Item/ItemBox.h"
#include "ItemData.h"
#include "Item/ItemDataManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Item/BaseItem.h"
#include "PlayerCharacter/THPlayerCharacter.h"


AItemBox::AItemBox()
{
	PrimaryActorTick.bCanEverTick = true;
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;

    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(RootComponent); // ItemMesh에 붙이기

    OverlapSphere->InitSphereRadius(100.f); // 원하는 반경
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    
}

void AItemBox::BeginPlay()
{
	Super::BeginPlay();
    if (OverlapSphere)
    {
        OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemBox::OnOverlapBegin);
        OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &AItemBox::OnOverlapEnd);
        UE_LOG(LogTemp, Warning, TEXT("OverlapSphere is valid and OnComponentBeginOverlap is bound."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OverlapSphere is null!"));
	}
}

void AItemBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemBox::OpenBox()
{
    if (UseTimeCheck)
    {
        return;
    }
	UseTimeCheck = true;
	//타이머 설정
	GetWorld()->GetTimerManager().SetTimer(UseTimerHandle, this, &AItemBox::ResetUseTime, 0.3f, false);


	//아이템 생성 및 드랍
	FString RandomItemID = RandomItemGenerate(EItemType::Equipment);
	DropItem(RandomItemID);
    
    
    //상자 삭제
	//Destroy();
}

void AItemBox::ResetUseTime()
{
    UseTimeCheck = false;
}


FString AItemBox::RandomItemGenerate(EItemType DropType)
{
    AItemDataManager* DataManager = Cast<AItemDataManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AItemDataManager::StaticClass()));

    if (!IsValid(DataManager))
    {
        UE_LOG(LogTemp, Error, TEXT("ItemDataManager is not valid."));
        return TEXT("Invalid");
    }

    TArray<const FItemData*> FilteredItems;
    int32 TotalWeight = 0;

    TArray<FItemData*> AllItemData;

    //DataManager->ItemDataTable이 유효한지 체크
    if (!IsValid(DataManager->ItemDataTable))
    {
        UE_LOG(LogTemp, Error, TEXT("ItemDataTable is not valid."));
        return TEXT("Invalid");
	}

    DataManager->ItemDataTable->GetAllRows(TEXT(""), AllItemData);

    for (const FItemData* ItemData : AllItemData)
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
        UE_LOG(LogTemp, Warning, TEXT("No valid items to drop or total weight is zero."));
        return TEXT("Invalid");
    }

    int32 RandomValue = FMath::RandRange(1, TotalWeight);
    int32 CurrentWeight = 0;

    for (const FItemData* SelectedItem : FilteredItems)
    {
        CurrentWeight += static_cast<int32>(SelectedItem->DropWeight);
        if (RandomValue <= CurrentWeight)
        {
            return SelectedItem->ItemID;
        }
    }

    return TEXT("Invalid");
}


void AItemBox::DropItem(FString RandomItemID)
{	
    if (RandomItemID == TEXT("Invalid"))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get a valid ItemID."));
        return;
    }

    AItemDataManager* DataManager = Cast<AItemDataManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AItemDataManager::StaticClass()));
	if (!IsValid(DataManager))
	{
		UE_LOG(LogTemp, Error, TEXT("ItemDataManager subsystem is not valid."));
		return;
	}

    const FItemData* ItemData = nullptr;
    TArray<FItemData*> AllRows;
    DataManager->ItemDataTable->GetAllRows(TEXT(""), AllRows);

    for (const FItemData* Row : AllRows)
    {
        if (Row && Row->ItemID == RandomItemID)
        {
            ItemData = Row;
            break;
        }
    }

    if (!ItemData || !IsValid(ItemData->BaseItemClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to find ItemData or BaseItemClass for ItemID: %s"), *RandomItemID);
        return;
    }

    FVector Location = GetActorLocation();
    FRotator Rotation = GetActorRotation();

	Location.Z += DropHeight; // 드랍 높이 조정

    ABaseItem* DroppedItem = GetWorld()->SpawnActor<ABaseItem>(ItemData->BaseItemClass, Location, Rotation);
    if (DroppedItem)
    {
        DroppedItem->SetItemID(RandomItemID);
        UE_LOG(LogTemp, Log, TEXT("Dropped Item ID: %s"), *RandomItemID);
    }
}

void AItemBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{   
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    InteractPromptWidget = CreateWidget<UInteractPromptWidget>(GetWorld(), InteractPromptClass);
    if (InteractPromptWidget)
    {
        InteractPromptWidget->AddToViewport();
    }

    ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
    if (PlayerChar)
    {
        // 플레이어의 함수를 호출하여 이 상자를 상호작용 가능한 대상으로 설정합니다.
        PlayerChar->SetInteractableActor(this);
    }
}

void AItemBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn && InteractPromptWidget)
    {
        // UI를 뷰포트에서 제거합니다.
        InteractPromptWidget->RemoveFromParent();

        ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
        if (PlayerChar)
        {
            // 플레이어의 함수를 호출하여 이 상자를 상호작용 가능한 대상으로 설정합니다.
            PlayerChar->SetInteractableActor(nullptr);
        }
    }
}


