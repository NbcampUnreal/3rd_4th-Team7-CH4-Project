#include "Item/THBaseItem.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Item/THItemInventory.h"
#include "Net/UnrealNetwork.h"


ATHBaseItem::ATHBaseItem()
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

	GetWorld()->GetTimerManager().SetTimer(TurningTimerHandle, this, &ATHBaseItem::TurningItem, 0.02f, true);
}

void ATHBaseItem::TurningItem()
{
	FRotator NewRotation = FRotator(0, RotationSpeed, 0);
	AddActorLocalRotation(NewRotation);
}

void ATHBaseItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TurningTimerHandle);
	}

}


void ATHBaseItem::SetItemID(FName NewItemID)
{
	ItemID = NewItemID;
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

	APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
	if (!PC) return;

	// 이미 생성되어 있는지 확인
	if (!InteractPromptWidgets.Contains(PC) && InteractPromptClass)
	{
		UTHInteractPromptWidget* NewWidget = CreateWidget<UTHInteractPromptWidget>(PC, InteractPromptClass);
		if (NewWidget)
		{
			NewWidget->AddToViewport();
			InteractPromptWidgets.Add(PC, NewWidget);
		}
	}

	PlayerChar->SetInteractableBaseItem(this);
}


void ATHBaseItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATHPlayerCharacter* PlayerChar = Cast<ATHPlayerCharacter>(OtherActor);
	if (!PlayerChar || !PlayerChar->IsLocallyControlled()) return;

	APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
	if (!PC) return;

	UTHInteractPromptWidget** WidgetPtr = InteractPromptWidgets.Find(PC);
	if (WidgetPtr && *WidgetPtr)
	{
		(*WidgetPtr)->RemoveFromParent();
		*WidgetPtr = nullptr;
		InteractPromptWidgets.Remove(PC);
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

	return false;
}


void ATHBaseItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATHBaseItem, bIsPickedUp);
}


