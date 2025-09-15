#include "ParkourComponent/THParkourComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

static TAutoConsoleVariable<int32> DebugMantle(
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
	
	const bool bDrawDebug = DebugMantle.GetValueOnGameThread() > 0;
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);

	const FVector StartLocation = OwnerCharacter->GetActorLocation();
	const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector FrontTraceStart = (StartLocation + (ForwardVector * CapsuleRadius)) + FVector(0, 0, CapsuleHalfHeight);
	const FVector FrontTraceEnd = FrontTraceStart + ForwardVector * MantleTraceDistance;

	FHitResult FrontHit;
	const bool bFrontHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		FrontTraceStart,
		FrontTraceEnd,
		CapsuleRadius * 0.7f,
		TraceChannel,
		false,
		{ OwnerCharacter },
		bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		FrontHit,
		true
	);

	if (!bFrontHit)
	{
		return false;
	}
    
	const FVector SurfaceTraceStart = FrontHit.ImpactPoint + (ForwardVector * 10.f) + FVector(0, 0, MaxMantleHeight);
	const FVector SurfaceTraceEnd = FVector(SurfaceTraceStart.X, SurfaceTraceStart.Y, StartLocation.Z);

	FHitResult SurfaceHit;
	const bool bSurfaceHit = UKismetSystemLibrary::LineTraceSingle(
		this,
		SurfaceTraceStart,
		SurfaceTraceEnd,
		TraceChannel,
		false,
		{ OwnerCharacter },
		bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		SurfaceHit,
		true
	);

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
    
	const FVector LedgeLocation = SurfaceHit.ImpactPoint;
	const FVector TargetLocation = LedgeLocation + FVector(0.f, 0.f, CapsuleHalfHeight);
	const FVector BoxTraceLocation = TargetLocation;
	const FVector BoxHalfSize = FVector(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);
    
    FHitResult BoxHit;
    const bool bBoxHit = UKismetSystemLibrary::BoxTraceSingle(
        this, 
        BoxTraceLocation, 
        BoxTraceLocation, 
        BoxHalfSize,
        FRotator::ZeroRotator, 
        TraceChannel, 
        false, 
        {OwnerCharacter}, 
        bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        BoxHit, 
        true
    );
    
	if (bBoxHit)
	{
		if (bDrawDebug)
		{
       		DrawDebugBox(GetWorld(), BoxTraceLocation, BoxHalfSize, FColor::Red, false, 5.f);
		}
    		return false;
    }
    
	const FRotator TargetRotation = FRotationMatrix::MakeFromX(-FrontHit.ImpactNormal).Rotator();

	OutMantleInfo.LedgeLocation = LedgeLocation;
	OutMantleInfo.TargetLocation = TargetLocation;
	OutMantleInfo.TargetRotation = TargetRotation;
	OutMantleInfo.TargetComponent = SurfaceHit.GetComponent();

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), OutMantleInfo.TargetLocation, 15.f, 16, FColor::Purple, false, 5.f);
	}

	return true;
}
