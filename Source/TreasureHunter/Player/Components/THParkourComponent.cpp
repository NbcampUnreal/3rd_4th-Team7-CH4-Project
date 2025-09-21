#include "THParkourComponent.h"
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

bool UTHParkourComponent::CheckMantle(FMantleInfo& OutMantleInfo) const
{
    if (!IsValid(OwnerCharacter) || !IsValid(OwnerMovementComponent))
    {
       return false;
    }
	
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const FVector ActorForward = OwnerCharacter->GetActorForwardVector();
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	FHitResult FrontHit;
	const FVector FrontTraceStart = ActorLocation + FVector(0, 0, 50.f);
	const FVector FrontTraceEnd = FrontTraceStart + ActorForward * MantleReachDistance;
	const float FrontTraceRadius = CapsuleRadius - 2.f;

	UKismetSystemLibrary::SphereTraceSingle(this, FrontTraceStart, FrontTraceEnd, FrontTraceRadius, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, DrawDebugType, FrontHit, true);

	if (!FrontHit.bBlockingHit || FrontHit.ImpactNormal.Z > 0.1f)
	{
		return false;
	}

	FHitResult SurfaceHit;
	const FVector DownwardTraceStart = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z + MaxMantleHeight);
	const FVector DownwardTraceEnd = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z);

	UKismetSystemLibrary::LineTraceSingle(this, DownwardTraceStart, DownwardTraceEnd, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, DrawDebugType, SurfaceHit, true);
	
	if (!SurfaceHit.bBlockingHit)
	{
		return false;
	}

	const float MantleHeight = SurfaceHit.ImpactPoint.Z - ActorLocation.Z;
	if (MantleHeight < MinMantleHeight)
	{
		return false;
	}

	const FVector BoxTraceLocation = SurfaceHit.ImpactPoint + FVector(0, 0, CapsuleHalfHeight + 1.f);
	const FVector BoxHalfSize(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);
	FHitResult BoxHit;

	const bool bIsSpaceBlocked = UKismetSystemLibrary::BoxTraceSingle(this, BoxTraceLocation, BoxTraceLocation, BoxHalfSize, FRotator::ZeroRotator, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, DrawDebugType, BoxHit, true);

	if (bIsSpaceBlocked)
	{
		return false;
	}
	
	const FVector LedgeLocation = SurfaceHit.ImpactPoint;
	const FRotator WallNormalRotation = FRotationMatrix::MakeFromX(-FrontHit.ImpactNormal).Rotator();
	const FVector InwardDirection = -FrontHit.ImpactNormal;

	const FVector ProjectedLocationOnWall = UKismetMathLibrary::ProjectPointOnToPlane(ActorLocation, FrontHit.ImpactPoint, FrontHit.ImpactNormal);

	FVector UpTargetLocation = FVector(ProjectedLocationOnWall.X, ProjectedLocationOnWall.Y, LedgeLocation.Z);
	UpTargetLocation += InwardDirection * UpWarpTargetInwardOffset;
	UpTargetLocation.Z += FinalLandingHeightOffset;
	OutMantleInfo.UpWarpTarget = FTransform(WallNormalRotation, UpTargetLocation, FVector::OneVector);

	FVector FinalLocation = FVector(ProjectedLocationOnWall.X, ProjectedLocationOnWall.Y, LedgeLocation.Z);
	const float TotalInwardOffset = CapsuleRadius + MantleForwardOffset;
	FinalLocation += InwardDirection * TotalInwardOffset;
	FinalLocation.Z += FinalLandingHeightOffset;
	OutMantleInfo.ForwardWarpTarget = FTransform(WallNormalRotation, FinalLocation, FVector::OneVector);
	
	OutMantleInfo.LedgeLocation = LedgeLocation;
	OutMantleInfo.TargetComponent = SurfaceHit.GetComponent();

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), OutMantleInfo.UpWarpTarget.GetLocation(), 15.f, 16, FColor::Yellow, false, 5.f);
		DrawDebugSphere(GetWorld(), OutMantleInfo.ForwardWarpTarget.GetLocation(), 15.f, 16, FColor::Cyan, false, 5.f);
	}
	
	return true;
}