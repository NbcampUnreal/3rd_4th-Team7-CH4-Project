#include "Item/Spawn/THRocketMouse.h"
#include "Item/Spawn/THRocketMouseBomb.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Misc/DateTime.h"
#include "GameFramework/Character.h"


ATHRocketMouse::ATHRocketMouse()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // ��Ʈ �浹 ������Ʈ
    USphereComponent* CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->InitSphereRadius(50.f);
    RootComponent = CollisionComponent;

    // �ʱ� �ӵ�
    InitialSpeed = 1500.f;
}

void ATHRocketMouse::BeginPlay()
{
    Super::BeginPlay();

    if (PlacerActor)
    {
        // ĳ���� ���� ���� XY
        FVector ForwardXY = PlacerActor->GetActorForwardVector();
        ForwardXY.Z = 0.f;
        ForwardXY.Normalize();

        // ���ϴ� ���Ⱒ
        
        float UpAngleRad = FMath::DegreesToRadians(UpAngleDeg);

        float Z = FMath::Tan(UpAngleRad) * ForwardXY.Size();

        FVector LaunchDir = ForwardXY;
        LaunchDir.Z = Z;
        LaunchDir.Normalize();

        InitialVelocity = LaunchDir * InitialSpeed;
    }

    // PlacerActor�� �غ�Ǹ� ȣ��
    FTimerHandle PlacerCheckTimer;
    GetWorldTimerManager().SetTimer(PlacerCheckTimer, this, &ATHRocketMouse::OnPlacerActorReady, SpawnDelay, false);



}

void ATHRocketMouse::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector NewLocation = GetActorLocation() + InitialVelocity * DeltaTime;
    SetActorLocation(NewLocation);

    SetActorRotation(InitialVelocity.Rotation());
}



void ATHRocketMouse::OnPlacerActorReady()
{
    if (!PlacerActor || !RocketMouseBombClass) return;

    AActor* TargetActorA = nullptr;
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundCharacters);

    for (AActor* ActorA : FoundCharacters)
    {
        ACharacter* CharacterA = Cast<ACharacter>(ActorA);
        if (CharacterA != PlacerActor && CharacterA->IsPlayerControlled())
        {
            TargetActorA = CharacterA;
            break;
        }
    }

    if (!TargetActorA)
    {
        Destroy();
        return;
    }

    FVector TargetLoc = TargetActorA->GetActorLocation();


    SetActorHiddenInGame(true);

    if (HasAuthority())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ATHRocketMouseBomb* Bomb = GetWorld()->SpawnActor<ATHRocketMouseBomb>(RocketMouseBombClass, TargetLoc + BombOffset, FRotator::ZeroRotator, SpawnParams);
        if (Bomb)
        {
            Bomb->SetTarget(TargetActorA);
        }
    }
    
    Destroy();
}



void ATHRocketMouse::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATHRocketMouse, InitialVelocity);
}


//void ATHRocketMouse::Multicast_ItemUseEffect_Implementation()
//{
//    UGameplayStatics::PlaySoundAtLocation(
//        GetWorld(),
//        EffectSound,
//        GetActorLocation(),
//        FRotator::ZeroRotator
//    );
//}