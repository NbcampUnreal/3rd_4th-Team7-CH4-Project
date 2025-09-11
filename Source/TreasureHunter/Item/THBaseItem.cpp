#include "Item/THBaseItem.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Item/THItemInventory.h"

ATHBaseItem::ATHBaseItem()
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

    bIsPickedUp = false;    
}

void ATHBaseItem::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATHBaseItem::OnOverlapBegin);
	OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ATHBaseItem::OnOverlapEnd);

    GetWorld()->GetTimerManager().SetTimer(PickedUpTimerHandle, this, &ATHBaseItem::PickedUpTime, 0.3f, false);
}

void ATHBaseItem::SetItemID(FString NewItemID)
{
	ItemID = NewItemID;
}

void ATHBaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATHBaseItem::PickedUpTime()
{
    bIsPickedUp = true;
}


void ATHBaseItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    InteractPromptWidget = CreateWidget<UTHInteractPromptWidget>(GetWorld(), InteractPromptClass);
    if (InteractPromptWidget)
    {
        InteractPromptWidget->AddToViewport();
    }

    ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
    if (PlayerChar)
    {
        // 플레이어의 함수를 호출하여 이 상자를 상호작용 가능한 대상으로 설정합니다.
        PlayerChar->SetInteractableBaseItem(this);
    }
}

void ATHBaseItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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
            PlayerChar->SetInteractableBaseItem(nullptr);
        }
    }
}

void ATHBaseItem::ItemPickup(ATHPlayerCharacter* PlayerCharacter)
{
    UE_LOG(LogTemp, Warning, TEXT("Item Pickup"));
    if (PlayerCharacter)
    {
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter is valid"));
        UTHItemInventory* Inventory = PlayerCharacter->FindComponentByClass<UTHItemInventory>();
        if (Inventory)
        {
			UE_LOG(LogTemp, Warning, TEXT("Inventory is valid"));
            if(Inventory->AddItem(ItemID))
            {
				UE_LOG(LogTemp, Warning, TEXT("Item %s picked up"), *ItemID);
                Destroy();
			}
        }
    }
}
