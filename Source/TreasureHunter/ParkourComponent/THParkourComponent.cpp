#include "ParkourComponent/THParkourComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

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

	const bool bDrawDebug = true;
	
	const FVector StartLocation = OwnerCharacter->GetActorLocation();
	const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	FHitResult FrontHit;
	const FVector FrontTraceStart = StartLocation + FVector(0, 0, CapsuleHalfHeight);
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);
	const bool bFrontHit = UKismetSystemLibrary::SphereTraceSingle(this, FrontTraceStart, FrontTraceStart + ForwardVector * MantleTraceDistance, CapsuleRadius * 0.7f,
		TraceChannel, false, {OwnerCharacter}, bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, FrontHit, true);

	if (!bFrontHit)
	{
		return false;
	}
	
	FHitResult SurfaceHit;
	const FVector SurfaceTraceStart = FrontHit.ImpactPoint + (ForwardVector * CapsuleRadius) + FVector(0, 0, MaxMantleHeight);
	const FVector SurfaceTraceEnd = FVector(SurfaceTraceStart.X, SurfaceTraceStart.Y, StartLocation.Z);
	const bool bSurfaceHit = UKismetSystemLibrary::LineTraceSingle(this, SurfaceTraceStart, SurfaceTraceEnd, TraceChannel,
		false, {OwnerCharacter}, bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, SurfaceHit, true);

	if (!bSurfaceHit)
	{
		return false;
	}
	
	const float MantleHeight = SurfaceHit.ImpactPoint.Z - StartLocation.Z;
	if (MantleHeight < MinMantleHeight || MantleHeight > MaxMantleHeight)
	{
		if (bDrawDebug)
		{
			DrawDebugSphere(GetWorld(), SurfaceHit.ImpactPoint, 10, 16, FColor::Red, false, 5.f);
		}
		return false;
	}

	const FVector TargetLocation = SurfaceHit.ImpactPoint;
	const FVector BoxTraceLocation = TargetLocation + FVector(0, 0, CapsuleHalfHeight);
	const FVector BoxHalfSize = FVector(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);

	FHitResult BoxHit;
	const bool bBoxHit = UKismetSystemLibrary::BoxTraceSingle(this, BoxTraceLocation, BoxTraceLocation, BoxHalfSize,
		FRotator::ZeroRotator, TraceChannel, false, {OwnerCharacter}, bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, BoxHit, true);

	if (bBoxHit)
	{
		if (bDrawDebug)
		{
			DrawDebugBox(GetWorld(), BoxTraceLocation, BoxHalfSize, FColor::Red, false, 5.f);
		}
		return false;
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), TargetLocation, 10, 16, FColor::Green, false, 5.f);
	}
	
	OutMantleInfo.LedgeLocation = SurfaceHit.ImpactPoint;
	OutMantleInfo.TargetLocation = TargetLocation + FVector(0, 0, 5.0f);
	OutMantleInfo.TargetRotation = FRotationMatrix::MakeFromX(-FrontHit.ImpactNormal).Rotator();
	OutMantleInfo.TargetComponent = SurfaceHit.GetComponent();

	return true;
}