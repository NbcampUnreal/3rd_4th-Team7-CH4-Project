#include "Item/THBaseItem.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Item/THItemInventory.h"
#include "Net/UnrealNetwork.h"


ATHBaseItem::ATHBaseItem()
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

    bIsPickedUp = false;
}

void ATHBaseItem::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATHBaseItem::OnOverlapBegin);
	OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ATHBaseItem::OnOverlapEnd);

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(PickedUpTimerHandle, this, &ATHBaseItem::EnablePickup, 0.3f, false);
	}
}

void ATHBaseItem::SetItemID(FName NewItemID)
{
	ItemID = NewItemID;
}

void ATHBaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATHBaseItem::EnablePickup()
{
	if (HasAuthority())
	{
		bIsPickedUp = true;
	}
}


void ATHBaseItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
	if (!PlayerChar || !PlayerChar->IsLocallyControlled()) return;

	if (!InteractPromptWidget && InteractPromptClass)
	{
		InteractPromptWidget = CreateWidget<UTHInteractPromptWidget>(GetWorld(), InteractPromptClass);
		if (InteractPromptWidget)
		{
			InteractPromptWidget->AddToViewport();
		}
	}
	
	PlayerChar->SetInteractableBaseItem(this);
}

void ATHBaseItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
	if (!PlayerChar || !PlayerChar->IsLocallyControlled()) return;

	if (InteractPromptWidget)
	{
		InteractPromptWidget->RemoveFromParent();
		InteractPromptWidget = nullptr;
	}

	PlayerChar->SetInteractableBaseItem(nullptr);
}



bool ATHBaseItem::ItemPickup(ATHPlayerCharacter* PlayerCharacter)
{
    if (!bIsPickedUp || !PlayerCharacter) return false;

    if (HasAuthority())
    {
        UTHItemInventory* Inventory = PlayerCharacter->FindComponentByClass<UTHItemInventory>();
        if (!Inventory) return false;

        if (Inventory->AddItem(ItemID))
        {
            Destroy();
            return true;
        }
        return false;
    }
    else
    {
        if (UTHItemInventory* Inventory = PlayerCharacter->FindComponentByClass<UTHItemInventory>())
        {
            Inventory->Server_AddItem(ItemID);
        }
        return false;
    }
}


void ATHBaseItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATHBaseItem, bIsPickedUp);
}


