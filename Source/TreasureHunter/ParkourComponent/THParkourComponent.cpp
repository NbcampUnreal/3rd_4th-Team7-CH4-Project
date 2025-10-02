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

	const bool bIsValidWall = OutFrontHit.bBlockingHit && FMath::Abs(OutFrontHit.ImpactNormal.Z) < 0.707f;
	
	return bIsValidWall;
}

bool UTHParkourComponent::TraceForLedge(const FHitResult& FrontHit, FHitResult& OutSurfaceHit) const
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();

	const FVector WallNormal = FrontHit.ImpactNormal;
	const FVector Offset = WallNormal * 5.f;

	const FVector DownwardTraceStart = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z + MaxMantleHeight) + Offset;
	const FVector DownwardTraceEnd = FVector(FrontHit.ImpactPoint.X, FrontHit.ImpactPoint.Y, ActorLocation.Z) + Offset;
	
	const float TraceRadius = 10.f;

	UKismetSystemLibrary::SphereTraceSingle(
		this,
		DownwardTraceStart,
		DownwardTraceEnd,
		TraceRadius,
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

bool UTHParkourComponent::IsLandingSpaceClear(const FVector& LandingLocation, const FRotator& TargetRotation) const // 수정된 함수
{
	const bool bDrawDebug = CVarDebugMantle.GetValueOnGameThread() > 0;
	const EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const TArray<AActor*> ActorsToIgnore = { OwnerCharacter };
	
	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	const FVector BoxHalfSize(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);
	FHitResult BoxHit;

	const bool bIsSpaceBlocked = UKismetSystemLibrary::BoxTraceSingle(this, 
		LandingLocation,
		LandingLocation, 
		BoxHalfSize, 
		TargetRotation, 
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic), 
		false, 
		ActorsToIgnore, 
		DrawDebugType, 
		BoxHit, 
		true);

	return !bIsSpaceBlocked;
}

void UTHParkourComponent::CalculateWarpTargets(const FHitResult& FrontHit, const FHitResult& SurfaceHit, FMantleInfo& OutMantleInfo) const
{
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector LedgeTopLocation = SurfaceHit.ImpactPoint;
	const FVector WallNormal = FrontHit.ImpactNormal;
	
	FVector InwardDirection = -WallNormal;
	InwardDirection.Z = 0.f;
	InwardDirection.Normalize();
	
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

	FHitResult FrontHit, SurfaceHit;
	if (!TraceForWall(FrontHit) || !TraceForLedge(FrontHit, SurfaceHit))
	{
		return false;
	}

	FVector InwardDirection = -FrontHit.ImpactNormal;
	InwardDirection.Z = 0.f;
	InwardDirection.Normalize();
	const FRotator TargetRotation = InwardDirection.Rotation();

	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	// --- 최종 수정: 실제 착지 위치와 동일한 로직으로 검사 위치 계산 ---
	// CalculateWarpTargets 함수가 계산할 최종 위치를 여기서 미리 계산합니다.
	const FVector LedgeTopLocation = SurfaceHit.ImpactPoint;

	// 이것이 맨틀 후 캐릭터의 발(루트)이 위치할 최종 지점입니다.
	FVector FinalLandingRootLocation = LedgeTopLocation + (InwardDirection * (CapsuleRadius + MantleForwardOffset));
	FinalLandingRootLocation.Z += FinalLandingHeightOffset;

	// BoxTrace는 캡슐의 '중심'을 기준으로 해야 하므로, 루트 위치에서 캡슐의 절반 높이만큼 올려줍니다.
	const FVector LandingCapsuleCenter = FinalLandingRootLocation + FVector(0, 0, CapsuleHalfHeight);

	if (!IsLandingSpaceClear(LandingCapsuleCenter, TargetRotation))
	{
		return false;
	}
	// --- 수정 완료 ---

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