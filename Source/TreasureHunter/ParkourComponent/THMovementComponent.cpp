#include "ParkourComponent/THMovementComponent.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void UTHMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTHMovementComponent, CurrentClimbableSurfaceNormal);
}

void UTHMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningPlayerCharacter = Cast<ATHPlayerCharacter>(CharacterOwner);
	if (OwningPlayerCharacter)
	{
		OwningPlayerAnimInstance = OwningPlayerCharacter->GetMesh()->GetAnimInstance();
		if (OwningPlayerAnimInstance)
		{
			OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UTHMovementComponent::OnClimbMontageEnded);
			OwningPlayerAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UTHMovementComponent::OnClimbMontageEnded);
		}
	}
}

void UTHMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTHMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == ECustomMovementMode::MOVE_Climb)
	{
		PhysClimb(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

void UTHMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);
		OnEnterClimbStateDelegate.ExecuteIfBound();
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);

		const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanStandRotation);

		StopMovementImmediately();
		OnExitClimbStateDelegate.ExecuteIfBound();
	}
}

float UTHMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return MaxClimbSpeed;
	}
	return Super::GetMaxSpeed();
}

float UTHMovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbAcceleration;
	}
	return Super::GetMaxAcceleration();
}

FVector UTHMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const
{
	if (IsFalling() && OwningPlayerAnimInstance && OwningPlayerAnimInstance->IsAnyMontagePlaying())
	{
		return RootMotionVelocity;
	}
	return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
}

TArray<FHitResult> UTHMovementComponent::DoCapsuleTraceMultiForObjects(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes) const
{
	TArray<FHitResult> OutHitResults;
	EDrawDebugTrace::Type DebugType = EDrawDebugTrace::None;
	if (bShowDebugShape)
	{
		DebugType = EDrawDebugTrace::ForOneFrame;
		if (bDrawPersistantShapes)
		{
			DebugType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this, Start, End, ClimbCapsuleTraceRadius, ClimbCapsuleTraceHalfHeight,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugType,
		OutHitResults,
		false
	);
	return OutHitResults;
}

FHitResult UTHMovementComponent::DoLineTraceSingleForObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes) const
{
	FHitResult OutHitResult;
	EDrawDebugTrace::Type DebugType = EDrawDebugTrace::None;
	if (bShowDebugShape)
	{
		DebugType = EDrawDebugTrace::ForOneFrame;
		if (bDrawPersistantShapes)
		{
			DebugType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this, Start, End,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugType,
		OutHitResult,
		false
	);
	return OutHitResult;
}

bool UTHMovementComponent::TraceClimbableSurfaces()
{
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();
	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiForObjects(Start, End);
	return !ClimbableSurfacesTracedResults.IsEmpty();
}

FHitResult UTHMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset, bool bShowDebugShape, bool bDrawPersistantShapes) const
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;
	return DoLineTraceSingleForObject(Start, End, bShowDebugShape, bDrawPersistantShapes);
}

bool UTHMovementComponent::CanStartClimbing()
{
	if (IsFalling()) return false;
	if (!TraceClimbableSurfaces()) return false;
	if (!TraceFromEyeHeight(100.f).bBlockingHit) return false;
	return true;
}

void UTHMovementComponent::StartClimbing()
{
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climb);
}

void UTHMovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Falling);
}

void UTHMovementComponent::PhysClimb(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	TraceClimbableSurfaces();
	ProcessClimbableSurfaceInfo();

	if (CheckShouldStopClimbing() || CheckHasReachedFloor())
	{
		StopClimbing();
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(DeltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(DeltaTime);

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * DeltaTime;
	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(DeltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	}

	SnapMovementToClimbableSurfaces(DeltaTime);

	if (CheckHasReachedLedge())
	{
		PlayClimbMontage(ClimbToTopMontage);
	}
}

void UTHMovementComponent::ProcessClimbableSurfaceInfo()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (ClimbableSurfacesTracedResults.IsEmpty()) return;

	for (const FHitResult& HitResult : ClimbableSurfacesTracedResults)
	{
		CurrentClimbableSurfaceLocation += HitResult.ImpactPoint;
		CurrentClimbableSurfaceNormal += HitResult.ImpactNormal;
	}

	CurrentClimbableSurfaceLocation /= ClimbableSurfacesTracedResults.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();
}

bool UTHMovementComponent::CheckShouldStopClimbing() const
{
	if (ClimbableSurfacesTracedResults.IsEmpty()) return true;

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff = FMath::RadiansToDegrees(FMath::Acos(DotResult));

	if (DegreeDiff <= 60.f)
	{
		return true;
	}

	return false;
}

bool UTHMovementComponent::CheckHasReachedFloor() const
{
	if (GetUnrotatedClimbVelocity().Z > -10.f) return false;

	const FVector DownVector = -UpdatedComponent->GetUpVector();
	const FVector StartOffset = DownVector * 80.f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + DownVector;

	TArray<FHitResult> PossibleFloorHits = DoCapsuleTraceMultiForObjects(Start, End);

	if (PossibleFloorHits.IsEmpty()) return false;

	for (const FHitResult& HitResult : PossibleFloorHits)
	{
		if (FVector::Parallel(-HitResult.ImpactNormal, FVector::UpVector))
		{
			return true;
		}
	}

	return false;
}

bool UTHMovementComponent::CanClimbDownLedge() const
{
	if (IsFalling()) return false;

	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Down = -UpdatedComponent->GetUpVector();

	const FVector WalkableSurfaceTraceStart = ComponentLocation + Forward * ClimbDownWalkableSurfaceTraceOffset;
	const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + Down * 100.f;

	FHitResult WalkableSurfaceHit = DoLineTraceSingleForObject(WalkableSurfaceTraceStart, WalkableSurfaceTraceEnd);
	if (WalkableSurfaceHit.bBlockingHit)
	{
		const FVector LedgeTraceStart = WalkableSurfaceHit.ImpactPoint + Forward * ClimbDownLedgeTraceOffset;
		const FVector LedgeTraceEnd = LedgeTraceStart + Down * 200.f;
		FHitResult LedgeHit = DoLineTraceSingleForObject(LedgeTraceStart, LedgeTraceEnd);
		if (!LedgeHit.bBlockingHit)
		{
			return true;
		}
	}
	return false;
}

bool UTHMovementComponent::CheckHasReachedLedge() const
{
	if (GetUnrotatedClimbVelocity().Z < 10.f) return false;

	FHitResult LedgeHit = TraceFromEyeHeight(100.f, 50.f);
	if (!LedgeHit.bBlockingHit)
	{
		const FVector WalkableSurfaceTraceStart = LedgeHit.TraceEnd;
		const FVector DownVector = -UpdatedComponent->GetUpVector();
		const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + DownVector * 100.f;
		FHitResult WalkableSurfaceHit = DoLineTraceSingleForObject(WalkableSurfaceTraceStart, WalkableSurfaceTraceEnd);
		if (WalkableSurfaceHit.bBlockingHit)
		{
			return true;
		}
	}
	return false;
}

FQuat UTHMovementComponent::GetClimbRotation(float DeltaTime) const
{
	const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();

	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return CurrentQuat;
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();
	return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5.f);
}

void UTHMovementComponent::SnapMovementToClimbableSurfaces(float DeltaTime) const
{
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ProjectedCharacterToSurface = (CurrentClimbableSurfaceLocation - ComponentLocation).ProjectOnTo(ComponentForward);
	const FVector SnapVector = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();
	UpdatedComponent->MoveComponent(SnapVector * DeltaTime * MaxClimbSpeed, UpdatedComponent->GetComponentQuat(), true);
}

void UTHMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if (bEnableClimb)
	{
		if (CanStartClimbing())
		{
			PlayClimbMontage(IdleToClimbMontage);
		}
		else if (CanClimbDownLedge())
		{
			PlayClimbMontage(ClimbDownLedgeMontage);
		}
		else
		{
			TryStartVaulting();
		}
	}
	else
	{
		StopClimbing();
	}
}

void UTHMovementComponent::RequestHopping()
{
	const FVector UnrotatedLastInputVector = UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), GetLastInputVector());
	const float DotResult = FVector::DotProduct(UnrotatedLastInputVector.GetSafeNormal(), FVector::UpVector);

	if (DotResult >= 0.9f)
	{
		HandleHopUp();
	}
	else if (DotResult <= -0.9f)
	{
		HandleHopDown();
	}
}

bool UTHMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climb;
}

FVector UTHMovementComponent::GetUnrotatedClimbVelocity() const
{
	return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), Velocity);
}

void UTHMovementComponent::TryStartVaulting()
{
	FVector VaultStartPosition, VaultLandPosition;
	if (CanStartVaulting(VaultStartPosition, VaultLandPosition))
	{
		SetMotionWarpingTarget(FName("VaultStartPoint"), VaultStartPosition);
		SetMotionWarpingTarget(FName("VaultLandPoint"), VaultLandPosition);
		StartClimbing();
		PlayClimbMontage(VaultingMontage);
	}
}

bool UTHMovementComponent::CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition) const
{
	if (IsFalling()) return false;

	OutVaultStartPosition = FVector::ZeroVector;
	OutVaultLandPosition = FVector::ZeroVector;
	
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentUp = UpdatedComponent->GetUpVector();

	for (int32 i = 0; i < 5; ++i)
	{
		const FVector Start = ComponentLocation + ComponentUp * 100.f + ComponentForward * (80.f * (i + 1));
		const FVector End = Start - ComponentUp * (100.f * (i + 1));
		FHitResult TraceHit = DoLineTraceSingleForObject(Start, End);

		if (i == 0 && TraceHit.bBlockingHit)
		{
			OutVaultStartPosition = TraceHit.ImpactPoint;
		}
		if (i == 4 && TraceHit.bBlockingHit)
		{
			OutVaultLandPosition = TraceHit.ImpactPoint;
		}
	}
	
	return !OutVaultStartPosition.IsZero() && !OutVaultLandPosition.IsZero();
}

void UTHMovementComponent::PlayClimbMontage(UAnimMontage* MontageToPlay) const
{
	if (!MontageToPlay || !OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void UTHMovementComponent::OnClimbMontageEnded(UAnimMontage* MontageToPlay, bool bInterrupted)
{
	if (MontageToPlay == IdleToClimbMontage || MontageToPlay == ClimbDownLedgeMontage)
	{
		StartClimbing();
		StopMovementImmediately();
	}
	else if (MontageToPlay == ClimbToTopMontage || MontageToPlay == VaultingMontage)
	{
		SetMovementMode(MOVE_Walking);
	}
}

void UTHMovementComponent::SetMotionWarpingTarget(const FName& InWarpingTargetName, const FVector& InTargetPosition) const
{
	if (!OwningPlayerCharacter) return;
	OwningPlayerCharacter->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocation(InWarpingTargetName, InTargetPosition);
}

void UTHMovementComponent::HandleHopUp() const
{
	FVector TargetPosition;
	if (CheckCanHopUp(TargetPosition))
	{
		SetMotionWarpingTarget(FName("HopUpTargetPoint"), TargetPosition);
		PlayClimbMontage(HopUpMontage);
	}
}

bool UTHMovementComponent::CheckCanHopUp(FVector& OutTargetPosition) const
{
	FHitResult HitResult = TraceFromEyeHeight(100.f, -20.f);
	if (HitResult.bBlockingHit)
	{
		OutTargetPosition = HitResult.ImpactPoint;
		return true;
	}
	return false;
}

void UTHMovementComponent::HandleHopDown() const
{
	FVector TargetPosition;
	if (CheckCanHopDown(TargetPosition))
	{
		SetMotionWarpingTarget(FName("HopDownTargetPoint"), TargetPosition);
		PlayClimbMontage(HopDownMontage);
	}
}

bool UTHMovementComponent::CheckCanHopDown(FVector& OutTargetPosition) const
{
	FHitResult HitResult = TraceFromEyeHeight(100.f, -300.f);
	if (HitResult.bBlockingHit)
	{
		OutTargetPosition = HitResult.ImpactPoint;
		return true;
	}
	return false;
}