#include "Item/Spawn/THSlowTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Item/ItemAbility/THSpeedSlowAbility.h"
#include "PlayerCharacter/THPlayerCharacter.h"



ATHSlowTrap::ATHSlowTrap()
{
	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(OverlapSphere);
	TrapMesh->SetupAttachment(RootComponent);
	OverlapSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATHSlowTrap::OnOverlapBegin);
}

void ATHSlowTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATHSlowTrap, bIsActive);
}

void ATHSlowTrap::BeginPlay()
{
	Super::BeginPlay();

	bIsActive = false;

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(ActivationTimerHandle, this, &ATHSlowTrap::ActivateTrap, 2.0f, false);
	}
}

void ATHSlowTrap::ActivateTrap()
{
	bIsActive = true;
}

void ATHSlowTrap::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsActive)
	{
		if (OtherActor && OtherActor->IsA<ACharacter>())
		{
			ApplySlowEffect(OtherActor);
			bIsActive = false;
			Destroy();
			return;
		}
	}
}

void ATHSlowTrap::ApplySlowEffect(AActor* TargetActor)
{	
	if (!HasAuthority())
	{
		return;
	}
	if (!TargetActor || !SlowAbilityClass) return;

	if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(TargetActor))
	{
		if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
		{
			const FGameplayAbilitySpec Spec(SlowAbilityClass, 1, INDEX_NONE, this);
			const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
			ASC->TryActivateAbility(Handle);
		}
	}
}

void ATHSlowTrap::OnPlacerActorReplicated()
{
	if (PlacerActor)
	{
		if (PlacerActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			TrapMesh->SetHiddenInGame(false, true);
		}
		else
		{
			TrapMesh->SetHiddenInGame(true, true);
		}
	}
	else
	{
		TrapMesh->SetHiddenInGame(true, true);
	}
}