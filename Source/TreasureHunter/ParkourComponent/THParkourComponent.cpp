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
	
	const FVector StartLocation = OwnerCharacter->GetActorLocation();
	const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	FHitResult FrontHit;
	const FVector FrontTraceStart = StartLocation + FVector(0, 0, CapsuleHalfHeight);
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);
	
	if (!UKismetSystemLibrary::SphereTraceSingle(this, FrontTraceStart, FrontTraceStart + ForwardVector * MantleTraceDistance, CapsuleRadius * 0.7f,
		TraceChannel, false, {OwnerCharacter}, EDrawDebugTrace::None, FrontHit, true))
	{
		return false;
	}
	
	FHitResult SurfaceHit;
	const FVector SurfaceTraceStart = FrontHit.ImpactPoint + (ForwardVector * CapsuleRadius) + FVector(0, 0, MaxMantleHeight);
	const FVector SurfaceTraceEnd = FVector(SurfaceTraceStart.X, SurfaceTraceStart.Y, StartLocation.Z);

	if (!UKismetSystemLibrary::LineTraceSingle(this, SurfaceTraceStart, SurfaceTraceEnd, TraceChannel,
		false, {OwnerCharacter}, EDrawDebugTrace::None, SurfaceHit, true))
	{
		return false;
	}
	
	const float MantleHeight = SurfaceHit.ImpactPoint.Z - StartLocation.Z;
	if (MantleHeight < MinMantleHeight || MantleHeight > MaxMantleHeight)
	{
		return false;
	}

	const FVector TargetLocation = SurfaceHit.ImpactPoint;
	const FVector BoxTraceLocation = TargetLocation + FVector(0, 0, CapsuleHalfHeight);
	
	FHitResult BoxHit;
	if (UKismetSystemLibrary::BoxTraceSingle(this, BoxTraceLocation, BoxTraceLocation, FVector(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight),
		FRotator::ZeroRotator, TraceChannel, false, {OwnerCharacter}, EDrawDebugTrace::None, BoxHit, true))
	{
		return false;
	}
	
	OutMantleInfo.LedgeLocation = SurfaceHit.ImpactPoint;
	OutMantleInfo.TargetLocation = TargetLocation + FVector(0, 0, 5.0f);
	OutMantleInfo.TargetRotation = FRotationMatrix::MakeFromX(-FrontHit.ImpactNormal).Rotator();
	OutMantleInfo.TargetComponent = SurfaceHit.GetComponent();

	return true;
}