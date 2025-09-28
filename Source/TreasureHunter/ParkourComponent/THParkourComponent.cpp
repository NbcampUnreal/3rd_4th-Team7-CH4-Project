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
	const float FrontTraceRadius = CapsuleRadius - 2.f;

	UKismetSystemLibrary::SphereTraceSingle(this, FrontTraceStart, FrontTraceEnd, FrontTraceRadius, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, DrawDebugType, OutFrontHit, true);

	return OutFrontHit.bBlockingHit && OutFrontHit.ImpactNormal.Z <= 0.1f;
}

bool UTHParkourComponent::TraceForLedge(const FHitResult& FrontHit, FHitResult& OutSurfaceHit) const
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };
	
	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	
	const FVector DownwardTraceStart = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z + MaxMantleHeight);
	const FVector DownwardTraceEnd = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z);

	UKismetSystemLibrary::LineTraceSingle(this, DownwardTraceStart, DownwardTraceEnd, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, DrawDebugType, OutSurfaceHit, true);
	
	const float MantleHeight = OutSurfaceHit.ImpactPoint.Z - ActorLocation.Z;
	return OutSurfaceHit.bBlockingHit && (MantleHeight >= MinMantleHeight);
}

bool UTHParkourComponent::IsLandingSpaceClear(const FHitResult& SurfaceHit) const
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };
	
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	
	const FVector BoxTraceLocation = SurfaceHit.ImpactPoint + FVector(0, 0, CapsuleHalfHeight + 1.f);
	const FVector BoxHalfSize(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);
	FHitResult BoxHit;

	const bool bIsSpaceBlocked = UKismetSystemLibrary::BoxTraceSingle(this, BoxTraceLocation, BoxTraceLocation, BoxHalfSize, FRotator::ZeroRotator, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, DrawDebugType, BoxHit, true);

	return !bIsSpaceBlocked;
}

void UTHParkourComponent::CalculateWarpTargets(const FHitResult& FrontHit, const FHitResult& SurfaceHit, FMantleInfo& OutMantleInfo) const
{
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector LedgeTopLocation = SurfaceHit.ImpactPoint;
	const FVector WallNormal = FrontHit.ImpactNormal;
	const FVector InwardDirection = -WallNormal;
	const FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(InwardDirection);

	const FVector HandPlacementPoint = LedgeTopLocation + (InwardDirection * MantleHandPlacementOffset);
	FVector UpTargetLocation = HandPlacementPoint + (WallNormal * CapsuleRadius);
	UpTargetLocation.Z = LedgeTopLocation.Z + MantleUpZOffset;
	OutMantleInfo.UpWarpTarget = FTransform(TargetRotation, UpTargetLocation);

	FVector ForwardTargetLocation = LedgeTopLocation + (InwardDirection * (CapsuleRadius + MantleForwardOffset));
	ForwardTargetLocation.Z += FinalLandingHeightOffset;
	OutMantleInfo.ForwardWarpTarget = FTransform(TargetRotation, ForwardTargetLocation);
}

bool UTHParkourComponent::CheckMantle(FMantleInfo& OutMantleInfo) const
{
    if (!IsValid(OwnerCharacter) || !IsValid(OwnerMovementComponent))
    {
       return false;
    }

	FHitResult FrontHit;
	if (!TraceForWall(FrontHit))
	{
		return false;
	}

	FHitResult SurfaceHit;
	if (!TraceForLedge(FrontHit, SurfaceHit))
	{
		return false;
	}

	if (!IsLandingSpaceClear(SurfaceHit))
	{
		return false;
	}

	CalculateWarpTargets(FrontHit, SurfaceHit, OutMantleInfo);
	
	OutMantleInfo.LedgeLocation = SurfaceHit.ImpactPoint;
	OutMantleInfo.TargetComponent = SurfaceHit.GetComponent();

	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), OutMantleInfo.UpWarpTarget.GetLocation(), 15.f, 16, FColor::Yellow, false, 5.f);
		DrawDebugSphere(GetWorld(), OutMantleInfo.ForwardWarpTarget.GetLocation(), 15.f, 16, FColor::Cyan, false, 5.f);
	}
	
	return true;
}