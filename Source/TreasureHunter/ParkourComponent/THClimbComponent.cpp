#include "ParkourComponent/THClimbComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<int32> CVarDebugClimb(
	TEXT("th.Debug.Climb"),
	0,
	TEXT("Enable/Disable Climb Debug Drawing.\n")
	TEXT("0: Disable\n")
	TEXT("1: Enable\n"),
	ECVF_Cheat
);

UTHClimbComponent::UTHClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ClimbableTraceChannel = ECC_WorldStatic;
}

void UTHClimbComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

bool UTHClimbComponent::TraceForWall(FHitResult& OutHit) const
{
	if (!OwnerCharacter) return false;

	const bool bDrawDebug = CVarDebugClimb.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	
	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	
	const FVector Start = OwnerCharacter->GetActorLocation() - (Forward * 5.f);
	const FVector FrontTraceEnd = Start + Forward * (ForwardTraceDistance + 5.f);

	TArray<AActor*> ActorsToIgnore = {OwnerCharacter};
	
	bool bFrontHit = UKismetSystemLibrary::SphereTraceSingle( 
		GetWorld(),
		Start,
		FrontTraceEnd,
		CapsuleRadius,
		UEngineTypes::ConvertToTraceType(ClimbableTraceChannel),
		false,
		ActorsToIgnore,
		DrawDebugType,
		OutHit,
		true
	);
	
	if (!bFrontHit)
	{
		return false;
	}
	
	UPrimitiveComponent* HitComponent = OutHit.GetComponent();
	const bool bHasClimbableTag = HitComponent && HitComponent->ComponentHasTag(FName("Climbable"));
	
	const bool bIsInDistanceRange = (OutHit.Distance >= MinClimbStartDistance && OutHit.Distance <= MaxClimbStartDistance);

	return bHasClimbableTag && bIsInDistanceRange;
}

bool UTHClimbComponent::TraceForLedge(const FHitResult& WallHit, FHitResult& OutLedgeHit) const
{
	const bool bDrawDebug = CVarDebugClimb.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	TArray<AActor*> ActorsToIgnore = {OwnerCharacter};

	const FVector TraceStartBase = WallHit.ImpactPoint + (WallHit.ImpactNormal * 5.f);
	const FVector LedgeTraceStart = FVector(TraceStartBase.X, TraceStartBase.Y, TraceStartBase.Z + LedgeTraceStartHeight);
	const FVector LedgeTraceEnd = FVector(TraceStartBase.X, TraceStartBase.Y, WallHit.ImpactPoint.Z - LedgeTraceDepth);
	
	return UKismetSystemLibrary::LineTraceSingle(
		GetWorld(), LedgeTraceStart, LedgeTraceEnd,
		UEngineTypes::ConvertToTraceType(ClimbableTraceChannel), false, ActorsToIgnore,
		DrawDebugType, OutLedgeHit, true
	);
}

bool UTHClimbComponent::CheckClimbableSurface(FClimbTraceResult& OutResult) const
{
	FHitResult WallHit;
	if (!TraceForWall(WallHit))
	{
		return false;
	}

	FHitResult LedgeHit;
	if (!TraceForLedge(WallHit, LedgeHit))
	{
		return false;
	}

	OutResult.bCanClimb = true;
	OutResult.LedgeLocation = LedgeHit.ImpactPoint;
	OutResult.WallLocation = WallHit.ImpactPoint;
	OutResult.WallNormal = WallHit.ImpactNormal; 

	return true;
}

bool UTHClimbComponent::IsOnValidClimbSurface(const FVector& HitLocation, const FVector& WallNormal) const
{
	if (!OwnerCharacter) return false;

	const bool bDrawDebug = CVarDebugClimb.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	TArray<AActor*> ActorsToIgnore = {OwnerCharacter};
	
	const FVector CharacterRightVector = OwnerCharacter->GetActorRightVector();
	
	const float CheckSphereRadius = 5.f;
	
	const FVector RightCheckStart = HitLocation + CharacterRightVector * EdgeCheckOffset - WallNormal * 10.f;
	const FVector RightCheckEnd = RightCheckStart + WallNormal * 20.f;
	FHitResult RightHit;
	bool bRightHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), RightCheckStart, RightCheckEnd, CheckSphereRadius, UEngineTypes::ConvertToTraceType(ClimbableTraceChannel), false, ActorsToIgnore, DrawDebugType, RightHit, true);
	
	const FVector LeftCheckStart = HitLocation - CharacterRightVector * EdgeCheckOffset - WallNormal * 10.f;
	const FVector LeftCheckEnd = LeftCheckStart + WallNormal * 20.f;
	FHitResult LeftHit;
	bool bLeftHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), LeftCheckStart, LeftCheckEnd, CheckSphereRadius, UEngineTypes::ConvertToTraceType(ClimbableTraceChannel), false, ActorsToIgnore, DrawDebugType, LeftHit, true);

	return bLeftHit && bRightHit;
}

bool UTHClimbComponent::FindClimbableSurface(const FVector& DesiredDirection, FClimbTraceResult& OutResult) const
{
	if (!OwnerCharacter) return false;

	const ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(OwnerCharacter);
	if (!PlayerCharacter) return false;

	const bool bDrawDebug = CVarDebugClimb.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector WallNormal = PlayerCharacter->ClimbingWallNormal;

	const FVector SideLocation = OwnerCharacter->GetActorLocation() + DesiredDirection * SidewaysTraceOffset;
	const FVector TraceStart = SideLocation + WallNormal * (CapsuleRadius + 5.f);
	const FVector TraceEnd = SideLocation - WallNormal * ForwardTraceDistance;
	
	FHitResult WallHit;
	TArray<AActor*> ActorsToIgnore = { OwnerCharacter };
	
	bool bHitWall = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(), 
		TraceStart, 
		TraceEnd, 
		CapsuleRadius,
		UEngineTypes::ConvertToTraceType(ClimbableTraceChannel), 
		false, 
		ActorsToIgnore,
		DrawDebugType, 
		WallHit, 
		true
	);
	
	if (bHitWall)
	{
		UPrimitiveComponent* HitComponent = WallHit.GetComponent();
		if (HitComponent && HitComponent->ComponentHasTag(FName("Climbable")))
		{
			OutResult.bCanClimb = true;
			OutResult.LedgeLocation = WallHit.ImpactPoint; 
			OutResult.WallLocation = WallHit.ImpactPoint;
			OutResult.WallNormal = WallHit.ImpactNormal;
			return true;
		}
	}
	
	return false;
}