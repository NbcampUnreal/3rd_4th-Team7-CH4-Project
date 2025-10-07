#include "ParkourComponent/THParkourComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<int32> CVarDebugMantle(
	TEXT("th.Debug.Mantle"),
	0,
	TEXT("Enable/Disable Mantle Debug Drawing.\n")
	TEXT("0: Disable\n")
	TEXT("1: Enable\n"),
	ECVF_Cheat
);

UTHParkourComponent::UTHParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTHParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

bool UTHParkourComponent::TraceForWall(FHitResult& OutFrontHit) const
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const FVector ActorForward = OwnerCharacter->GetActorForwardVector();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	const FVector FrontTraceStart = ActorLocation + FVector(0, 0, 50.f);
	const FVector FrontTraceEnd = FrontTraceStart + ActorForward * MantleReachDistance;

	TArray<FHitResult> OutHits;
	UKismetSystemLibrary::SphereTraceMulti(
		this,
		FrontTraceStart,
		FrontTraceEnd,
		CapsuleRadius - 2.f,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false,
		ActorsToIgnore,
		DrawDebugType,
		OutHits,
		true
	);

	if (OutHits.IsEmpty())
	{
		return false;
	}

	FHitResult BestHit;
	float BestHitDot = -1.f;

	for (const FHitResult& Hit : OutHits)
	{
		if (Hit.bBlockingHit && FMath::Abs(Hit.ImpactNormal.Z) < 0.707f)
		{
			float DotProduct = FVector::DotProduct(ActorForward, -Hit.ImpactNormal);
			if (DotProduct > BestHitDot)
			{
				BestHitDot = DotProduct;
				BestHit = Hit;
			}
		}
	}

	if (BestHit.bBlockingHit)
	{
		OutFrontHit = BestHit;
		return true;
	}

	return false;
}

bool UTHParkourComponent::TraceForLedge(const FHitResult& FrontHit, FHitResult& OutSurfaceHit) const
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const FVector ActorForward = OwnerCharacter->GetActorForwardVector();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	const FVector TraceStartXY = ActorLocation + (ActorForward * (CapsuleRadius + 20.f));
	const FVector DownwardTraceStart = FVector(TraceStartXY.X, TraceStartXY.Y, ActorLocation.Z + MaxMantleHeight);
	const FVector DownwardTraceEnd = FVector(DownwardTraceStart.X, DownwardTraceStart.Y, ActorLocation.Z);

	UKismetSystemLibrary::SphereTraceSingle(
		this,
		DownwardTraceStart,
		DownwardTraceEnd,
		15.f,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false,
		ActorsToIgnore,
		DrawDebugType,
		OutSurfaceHit,
		true
	);
	
	const float MantleHeight = OutSurfaceHit.ImpactPoint.Z - ActorLocation.Z;
	const bool bIsSurfaceWalkable = OutSurfaceHit.ImpactNormal.Z > 0.7f;

	return OutSurfaceHit.bBlockingHit && (MantleHeight >= MinMantleHeight) && bIsSurfaceWalkable;
}

bool UTHParkourComponent::IsLandingSpaceClear(const FVector& LandingCapsuleCenter) const
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };
	
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	FHitResult CapsuleHit;
	const bool bIsSpaceBlocked = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		LandingCapsuleCenter,
		LandingCapsuleCenter,
		CapsuleRadius,
		CapsuleHalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false,
		ActorsToIgnore,
		DrawDebugType,
		CapsuleHit,
		true
	);

	return !bIsSpaceBlocked;
}

bool UTHParkourComponent::CheckMantle(FMantleInfo& OutMantleInfo) const
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };

	if (!IsValid(OwnerCharacter) || !IsValid(OwnerMovementComponent))
	{
		return false;
	}

	FHitResult FrontHit, SurfaceHit;
	if (!TraceForWall(FrontHit) || !TraceForLedge(FrontHit, SurfaceHit))
	{
		return false;
	}
	
	const FVector LedgeTopLocation = SurfaceHit.ImpactPoint;
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	
	FVector MantleDirection = OwnerCharacter->GetActorForwardVector();
	MantleDirection.Z = 0.f;
	MantleDirection.Normalize();

	FVector DesiredLandingXY = LedgeTopLocation + (MantleDirection * (CapsuleRadius + MantleForwardOffset));
	
	FVector TraceStart = FVector(DesiredLandingXY.X, DesiredLandingXY.Y, LedgeTopLocation.Z + MaxMantleHeight);
	FVector TraceEnd = FVector(DesiredLandingXY.X, DesiredLandingXY.Y, LedgeTopLocation.Z - CapsuleHalfHeight * 2.f);

	FHitResult GroundHit;
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		TraceStart,
		TraceEnd,
		10.f,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false,
		ActorsToIgnore,
		DrawDebugType,
		GroundHit,
		true
	);

	if (!GroundHit.bBlockingHit || GroundHit.ImpactNormal.Z < 0.7f)
	{
		return false;
	}

	FVector LandingRootLocationForCheck = GroundHit.ImpactPoint;
	LandingRootLocationForCheck.Z += 10.f; 
	const FVector LandingCapsuleCenterForCheck = LandingRootLocationForCheck + FVector(0, 0, CapsuleHalfHeight);

	if (!IsLandingSpaceClear(LandingCapsuleCenterForCheck))
	{
		if (bDrawDebug)
		{
			DrawDebugSphere(GetWorld(), LandingCapsuleCenterForCheck, CapsuleRadius, 16, FColor::Red, false, 2.f);
		}
		return false;
	}

	FVector FinalLandingRootLocation = GroundHit.ImpactPoint;
	FinalLandingRootLocation.Z += FinalLandingHeightOffset;
	
	CalculateWarpTargets(FrontHit, SurfaceHit, FinalLandingRootLocation, OutMantleInfo);
	
	OutMantleInfo.LedgeLocation = SurfaceHit.ImpactPoint;
	OutMantleInfo.TargetComponent = SurfaceHit.GetComponent();

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), OutMantleInfo.UpWarpTarget.GetLocation(), 15.f, 16, FColor::Yellow, false, 5.f);
		DrawDebugSphere(GetWorld(), OutMantleInfo.ForwardWarpTarget.GetLocation(), 15.f, 16, FColor::Cyan, false, 5.f);
	}
	
	return true;
}

void UTHParkourComponent::CalculateWarpTargets(const FHitResult& FrontHit, const FHitResult& SurfaceHit, const FVector& FinalLandingLocation, FMantleInfo& OutMantleInfo) const
{
	const FVector LedgeTopLocation = SurfaceHit.ImpactPoint;
	
	FVector MantleDirection = OwnerCharacter->GetActorForwardVector();
	MantleDirection.Z = 0.f;
	MantleDirection.Normalize();
	const FRotator TargetRotation = MantleDirection.Rotation();

	const FVector HandPlacementPoint = LedgeTopLocation + (MantleDirection * MantleHandPlacementOffset);
	FVector UpTargetLocation = HandPlacementPoint - (FrontHit.ImpactNormal * (MantleHandPlacementOffset / 2.f));
	UpTargetLocation.Z = LedgeTopLocation.Z + MantleUpZOffset;
	OutMantleInfo.UpWarpTarget = FTransform(TargetRotation, UpTargetLocation);

	OutMantleInfo.ForwardWarpTarget = FTransform(TargetRotation, FinalLandingLocation);
}