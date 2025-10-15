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
    bReplicates = true;
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;

    BoxMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BoxMesh"));    
    BoxMesh->SetupAttachment(ItemMesh);


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

    if (IdleAnim && BoxMesh)
    {
        BoxMesh->PlayAnimation(IdleAnim, true);
    }

    if (BoxMesh)
    {
        FRotator Rot = BoxMesh->GetRelativeRotation();
        Rot.Yaw += 180.f;
        BoxMesh->SetRelativeRotation(Rot);
    }
}



void ATHItemBox::ResetUseTime()
{
    UseTimeCheck = false;
}


FName ATHItemBox::RandomItemGenerate(ATHPlayerController* PC)
{
    ATHItemDataManager* DataManager = Cast<ATHItemDataManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ATHItemDataManager::StaticClass()));

    if (!IsValid(DataManager) || !IsValid(DataManager->ItemDataTable))
    {
        return FName("Invalid");
    }

    EItemType DropType;
    if(DataManager->WhoWinner(PC))
    {
        DropType = EItemType::Winner;
    }
    else
    {
        DropType = EItemType::Loser;
	}



    TArray<FName> FilteredRowNames;
    int32 TotalWeight = 0;

    TArray<FName> RowNames = DataManager->ItemDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        const FTHItemData* ItemData = DataManager->GetItemDataByRow(RowName);
        if (!ItemData) continue;

        if ((DropType == EItemType::Loser && ItemData->ItemDropType == EItemType::Winner) ||
            (DropType == EItemType::Winner && ItemData->ItemDropType == EItemType::Loser))
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
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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
    if (UseTimeCheck)
    {
        return;
    }
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

void ATHItemBox::OpenBox(ATHPlayerController* PC)
{
    if (!HasAuthority()) return;

    if (!UseTimeCheck)
    {
        UseTimeCheck = true;        

        FName RandomItemID = RandomItemGenerate(PC);
        DropItem(RandomItemID);

        Multicast_OpenBox();
    }
}

void ATHItemBox::Multicast_OpenBox_Implementation()
{
    OverlapDisable();

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        EffectSound,
        GetActorLocation(),
        FRotator::ZeroRotator
    );

    if (OpenAnim && BoxMesh)
    {
        BoxMesh->PlayAnimation(OpenAnim, false);

        float Duration = OpenAnim->GetPlayLength();
        GetWorldTimerManager().SetTimer(
            DestroyTimerHandle,
            FTimerDelegate::CreateUObject(this, &ATHItemBox::Multicast_PlayOpenIdle),
            Duration,
            false
        );
    }
    else
    {
        Multicast_DestroyBox();
    }
}


void ATHItemBox::Multicast_PlayOpenIdle_Implementation()
{
    if (OpenIdleAnim && BoxMesh)
    {
        BoxMesh->PlayAnimation(OpenIdleAnim, true);

        GetWorldTimerManager().SetTimer(
            DestroyTimerHandle,
            this,
            &ATHItemBox::Multicast_DestroyBox,
            OpenIdleDuration,
            false
        );
    }
    else
    {
        Multicast_DestroyBox();
    }
}










void ATHItemBox::Multicast_DestroyBox_Implementation()
{
    Destroy();
}

void ATHItemBox::PlayOpenIdle()
{
    if (OpenIdleAnim && BoxMesh)
    {
        BoxMesh->PlayAnimation(OpenIdleAnim, true);

        GetWorldTimerManager().SetTimer(
            DestroyTimerHandle,
            this,
            &ATHItemBox::Multicast_DestroyBox,
            OpenIdleDuration,
            false
        );
    }
    else
    {
        Multicast_DestroyBox();
    }
}



void ATHItemBox::OverlapDisable()
{
    OverlapSphere->SetCollisionResponseToChannel(
        ECollisionChannel::ECC_Pawn,
        ECollisionResponse::ECR_Ignore
    );
}