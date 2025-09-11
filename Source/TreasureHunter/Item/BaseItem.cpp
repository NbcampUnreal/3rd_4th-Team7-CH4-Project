#include "Item/BaseItem.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Item/ItemInventory.h"

ABaseItem::ABaseItem()
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

void ABaseItem::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnOverlapBegin);
	OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseItem::OnOverlapEnd);

    GetWorld()->GetTimerManager().SetTimer(PickedUpTimerHandle, this, &ABaseItem::PickedUpTime, 0.3f, false);
}

void ABaseItem::SetItemID(FString NewItemID)
{
	ItemID = NewItemID;
}

void ABaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ABaseItem::PickedUpTime()
{
    bIsPickedUp = true;
}


void ABaseItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
        PlayerChar->SetInteractableBaseItem(this);
    }
}

void ABaseItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

void ABaseItem::ItemPickup(ATHPlayerCharacter* PlayerCharacter)
{
    UE_LOG(LogTemp, Warning, TEXT("Item Pickup"));
    if (PlayerCharacter)
    {
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter is valid"));
        UItemInventory* Inventory = PlayerCharacter->FindComponentByClass<UItemInventory>();
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
