#include "Item/Spawn/THJumpPad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

ATHJumpPad::ATHJumpPad()
{
    bReplicates = true;
	
	JumpPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpPadMesh"));
	RootComponent = JumpPadMesh;

    JumpPadMesh->OnComponentBeginOverlap.AddDynamic(this, &ATHJumpPad::OnOverlapBegin);
    JumpPadMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}


void ATHJumpPad::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATHJumpPad, bIsActivated);
}

void ATHJumpPad::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character || !bIsActivated)
    {
        return;
    }

    float CharacterFeetZ = Character->GetActorLocation().Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    float JumpPadTopZ = this->GetActorLocation().Z + JumpPadMesh->Bounds.BoxExtent.Z;

    if (CharacterFeetZ > JumpPadTopZ - 10.f)
    {
        if (HasAuthority())
        {
            bIsActivated = false;
            Character->LaunchCharacter(FVector(0.f, 0.f, LaunchStrength), false, false);
            FTimerHandle DestroyTimerHandle;
            GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ATHJumpPad::DestroyJumpPad, 1.5f, false);
        }
        else
        {
            Server_ActivateJumpPad(Character);
        }
    }
}

void ATHJumpPad::Server_ActivateJumpPad_Implementation(ACharacter* CharacterToLaunch)
{
    if (!CharacterToLaunch || !bIsActivated)
    {
        return;
    }

    bIsActivated = false;
    CharacterToLaunch->LaunchCharacter(FVector(0.f, 0.f, LaunchStrength), false, false);

    FTimerHandle DestroyTimerHandle;
    GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ATHJumpPad::DestroyJumpPad, 1.5f, false);
}

bool ATHJumpPad::Server_ActivateJumpPad_Validate(ACharacter* CharacterToLaunch)
{
    return true;
}

void ATHJumpPad::DestroyJumpPad()
{
    Destroy();
}