#include "THSlowTrap.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Game/GameFlowTags.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameplayEffect.h"

ATHSlowTrap::ATHSlowTrap()
{
    bReplicates = true; // ATHSpawnObject에서 bOnlyRelevantToOwner=true
    PrimaryActorTick.bCanEverTick = false;

    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    SetRootComponent(OverlapSphere);
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    OverlapSphere->InitSphereRadius(60.f);

    TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
    TrapMesh->SetupAttachment(RootComponent);
    TrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATHSlowTrap::OnOverlapBegin);
}

void ATHSlowTrap::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        bIsActive = false;
        if (ActivateDelaySec > 0.f)
        {
            FTimerHandle TH;
            GetWorldTimerManager().SetTimer(TH, this, &ATHSlowTrap::ActivateTrap, ActivateDelaySec, false);
        }
        else
        {
            ActivateTrap();
        }
    }
}

void ATHSlowTrap::ActivateTrap()
{
    bIsActive = true;
}

void ATHSlowTrap::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
    if (!HasAuthority() || !bIsActive || !OtherActor) return;

    // 캡슐만 받으면 중복 호출 줄임
    if (!OtherComp || !OtherComp->IsA(UCapsuleComponent::StaticClass())) return;

    // 설치자 자신 무시
     //if (OtherActor == PlacerActor) return;


    if (Cast<ACharacter>(OtherActor))
    {
        ApplySlowTo(OtherActor);
        bIsActive = false;
        Destroy(); // 1회용
    }
}

void ATHSlowTrap::ApplySlowTo(AActor* TargetActor)
{
    if (!SlowGEClass) return;

    ATHPlayerCharacter* PC = Cast<ATHPlayerCharacter>(TargetActor);
    if (!PC) return;

    UAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
    if (!ASC) return;

    // 면역 태그 빠른 체크
    if (ImmunityTag.IsValid())
    {
        FGameplayTagContainer Owned;
        ASC->GetOwnedGameplayTags(Owned);
        if (Owned.HasTag(ImmunityTag)) return;
    }

    FGameplayEffectSpecHandle H = ASC->MakeOutgoingSpec(SlowGEClass, 1.f, ASC->MakeEffectContext());
    if (!H.IsValid()) return;

    FGameplayEffectSpec* Spec = H.Data.Get();
    check(Spec);

    Spec->SetSetByCallerMagnitude(TAG_Data_WalkDelta, WalkDelta);
    Spec->SetSetByCallerMagnitude(TAG_Data_SprintDelta, SprintDelta);

    ASC->ApplyGameplayEffectSpecToSelf(*Spec);
}

void ATHSlowTrap::InitFromItemRow(TSubclassOf<UGameplayEffect> InGE, float InWalkDelta, float InSprintDelta, float InDurationSec)
{
    SlowGEClass = InGE;
    WalkDelta = InWalkDelta;
    SprintDelta = InSprintDelta;
    DurationSec = InDurationSec; // 쓸거면 가져다 써도 됨. (일단 만들어만 둠 참고용)
}

void ATHSlowTrap::OnPlacerActorReplicated()
{
    if (TrapMesh)
    {
        TrapMesh->SetHiddenInGame(false, true);
    }
}

void ATHSlowTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATHSlowTrap, bIsActive);
}
