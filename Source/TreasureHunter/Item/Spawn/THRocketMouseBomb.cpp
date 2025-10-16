#include "Item/Spawn/THRocketMouseBomb.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystemComponent.h"
#include "PlayerCharacter/THPlayerCharacter.h"

ATHRocketMouseBomb::ATHRocketMouseBomb()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->InitSphereRadius(50.f);
    RootComponent = CollisionComponent;

    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ATHRocketMouseBomb::OnOverlapBegin);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->bShouldBounce = false;
}



void ATHRocketMouseBomb::OnRep_Velocity()
{
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = ReplicatedVelocity;
    }
}


void ATHRocketMouseBomb::SetTarget(AActor* NewTarget)
{
    TargetActor = NewTarget;
    if (TargetActor && ProjectileMovement)
    {
        ProjectileMovement->bIsHomingProjectile = true;
        ProjectileMovement->HomingTargetComponent = TargetActor->GetRootComponent();
        ProjectileMovement->HomingAccelerationMagnitude = FireSpeed;
    }
}

void ATHRocketMouseBomb::BeginPlay()
{
    Super::BeginPlay();
    
    Multicast_ItemUseEffect();
}



void ATHRocketMouseBomb::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (!OtherActor || OtherActor != TargetActor) return;

    Multicast_Explode();
    Destroy();
}

void ATHRocketMouseBomb::Multicast_Explode_Implementation()
{
    if (ExplosionNiagara)
    {
        ExplodeSound();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionNiagara, GetActorLocation(), GetActorRotation(), FVector(1.f), true, true);
        ApplyAbilityToTarget();
    }
}

void ATHRocketMouseBomb::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATHRocketMouseBomb, TargetActor);
    DOREPLIFETIME(ATHRocketMouseBomb, ReplicatedVelocity);
}

void ATHRocketMouseBomb::ApplyAbilityToTarget()
{
    if (!TargetActor || !ExplosionAbility) return;

    ATHPlayerCharacter* TargetCharacter = Cast<ATHPlayerCharacter>(TargetActor);
    if (!TargetCharacter) return;

    UAbilitySystemComponent* TargetASC = TargetCharacter->GetAbilitySystemComponent();
    if (!TargetASC) return;

    if (TargetASC->GetOwnerRole() == ROLE_Authority)
    {
        FGameplayAbilitySpec Spec(ExplosionAbility, 1, INDEX_NONE, TargetCharacter);
        FGameplayAbilitySpecHandle AbilityHandle = TargetASC->GiveAbility(Spec);
        TargetASC->TryActivateAbility(AbilityHandle);
    }
}

void ATHRocketMouseBomb::Multicast_ItemUseEffect_Implementation()
{
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        WingEffectSound,
        GetActorLocation(),
        FRotator::ZeroRotator
    );
}


void ATHRocketMouseBomb::ExplodeSound()
{
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        BombEffectSound,
        GetActorLocation(),
        FRotator::ZeroRotator
    );
}