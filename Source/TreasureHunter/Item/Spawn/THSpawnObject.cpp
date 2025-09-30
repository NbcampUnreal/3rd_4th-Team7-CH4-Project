#include "Item/Spawn/THSpawnObject.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ATHSpawnObject::ATHSpawnObject()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ATHSpawnObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATHSpawnObject, PlacerActor);
}

void ATHSpawnObject::SetPlacerActor(AActor* NewPlacer)
{
	if (HasAuthority())
	{
		PlacerActor = NewPlacer;
		OnPlacerActorReplicated();
	}
}

void ATHSpawnObject::OnRep_PlacerActor()
{
	OnPlacerActorReplicated();
}

void ATHSpawnObject::BeginPlay()
{
	Super::BeginPlay();
}

void ATHSpawnObject::OnPlacerActorReplicated()
{
	
}