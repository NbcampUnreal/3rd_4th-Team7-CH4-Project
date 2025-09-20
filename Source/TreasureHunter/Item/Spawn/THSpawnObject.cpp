#include "THSpawnObject.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ATHSpawnObject::ATHSpawnObject()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bOnlyRelevantToOwner = true;         // 설치자에게만 복제
	bNetUseOwnerRelevancy = false;       // 이거 하거나, 아니면 DOREPLIFETIME을 OwnerOnly로 하거나 
}

void ATHSpawnObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATHSpawnObject, PlacerActor);
}

void ATHSpawnObject::SetPlacerAndOwner(AActor* NewPlacer)
{
	if (!HasAuthority()) return;

	PlacerActor = NewPlacer;

	// Owner는 PlayerController가 가장 안전
	AController* PC = nullptr;
	if (APawn* Pawn = Cast<APawn>(NewPlacer))
	{
		PC = Pawn->GetController();
	}
	else
	{
		PC = Cast<AController>(NewPlacer);
	}
	if (PC) { SetOwner(PC); }
	else { SetOwner(NewPlacer); }
}

void ATHSpawnObject::OnRep_PlacerActor()
{
	OnPlacerActorReplicated();
}

void ATHSpawnObject::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnPlacerActorReplicated();
	}
}

void ATHSpawnObject::OnPlacerActorReplicated()
{
	
}