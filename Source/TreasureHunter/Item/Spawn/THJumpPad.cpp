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

	
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	JumpPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpPadMesh"));
	
	JumpPadMesh->SetupAttachment(RootComponent);
	JumpPadMesh->SetGenerateOverlapEvents(true);

    JumpPadMesh->OnComponentBeginOverlap.AddDynamic(this, &ATHJumpPad::OnOverlapBegin);
    JumpPadMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	JumpPadAniMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("JumpPadAniMesh"));
	JumpPadAniMesh->SetupAttachment(RootComponent);
	JumpPadAniMesh->SetCollisionProfileName(TEXT("NoCollision"));

}


void ATHJumpPad::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATHJumpPad, bIsActivated);
}

void ATHJumpPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
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
            JumpAction(Character);
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
    JumpAction(CharacterToLaunch);
}

bool ATHJumpPad::Server_ActivateJumpPad_Validate(ACharacter* CharacterToLaunch)
{
    return true;
}

void ATHJumpPad::DestroyJumpPad()
{
    Destroy();
}


void ATHJumpPad::JumpAction(ACharacter* CharacterToLaunch)
{
    bIsActivated = false;
    PlayJumpPadAnimation();
    CharacterToLaunch->LaunchCharacter(FVector(0.f, 0.f, LaunchStrength), false, false);
    FTimerHandle DestroyTimerHandle;
    GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ATHJumpPad::DestroyJumpPad, 1.5f, false);
}

void ATHJumpPad::PlayJumpPadAnimation()
{
    if (UAnimInstance* AnimInst = JumpPadAniMesh->GetAnimInstance())
    {
        if (JumpPadMontage)
        {
            AnimInst->Montage_Play(JumpPadMontage);
        }
    }

}


