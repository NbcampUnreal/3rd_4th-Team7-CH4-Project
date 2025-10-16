#include "ParkourComponent/THMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Game/GameFlowTags.h"
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
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(40.f);
		
		if (UAbilitySystemComponent* ASC = OwningPlayerCharacter->GetAbilitySystemComponent())
		{
			FGameplayTagContainer SprintAbilityTag;
			SprintAbilityTag.AddTag(TAG_Ability_Sprint);
			ASC->CancelAbilities(&SprintAbilityTag);
		}
		
		if (OwningPlayerCharacter)
		{
			OwningPlayerCharacter->bIsClimbing = true;
		}

		OnEnterClimbStateDelegate.ExecuteIfBound();
	}
	
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(80.f);

		const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanStandRotation);
		
		if (OwningPlayerCharacter)
		{
			OwningPlayerCharacter->bIsClimbing = false;
			OwningPlayerCharacter->ClimbMovementDirection = FVector2D::ZeroVector;
		}

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
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + Forward * 150.f;
	const FVector BoxHalfSize = FVector(10.f, 50.f, 70.f);
	const FRotator Orientation = UpdatedComponent->GetComponentRotation();
    
	TArray<AActor*> Ignore;
	Ignore.Add(CharacterOwner);

	ClimbableSurfacesTracedResults.Reset();

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		this, Start, End, BoxHalfSize, Orientation,
		ClimbableSurfaceTraceTypes, false, Ignore,
		EDrawDebugTrace::None, ClimbableSurfacesTracedResults, false
	);

	return !ClimbableSurfacesTracedResults.IsEmpty();
}

FHitResult UTHMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset, bool bShowDebugShape, bool bDrawPersistantShapes) const
{
	const UCapsuleComponent* Cap = CharacterOwner->GetCapsuleComponent();
	if (!ensure(Cap)) return FHitResult();

	const FVector Up = UpdatedComponent->GetUpVector();
	const FVector Fwd = UpdatedComponent->GetForwardVector();
	const float Half = Cap->GetScaledCapsuleHalfHeight();
	const float EyeOffsetFromTop = 10.f;
	const FVector Start = Cap->GetComponentLocation() + Up * (Half - EyeOffsetFromTop + TraceStartOffset);
	const FVector End = Start + Fwd * TraceDistance;

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

	if (CheckShouldStopClimbing())
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
		Multicast_PlayMontage(ClimbToTopMontage);
	}
}

void UTHMovementComponent::ProcessClimbableSurfaceInfo()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (ClimbableSurfacesTracedResults.IsEmpty()) return;
	
	FHitResult SteepestHitResult;
	float MinAbsDotZ = 1.0f;

	for (const FHitResult& HitResult : ClimbableSurfacesTracedResults)
	{
		const float CurrentDotZ = FMath::Abs(FVector::DotProduct(HitResult.ImpactNormal, FVector::UpVector));
		
		if (CurrentDotZ < MinAbsDotZ)
		{
			MinAbsDotZ = CurrentDotZ;
			SteepestHitResult = HitResult;
		}
	}
	
	CurrentClimbableSurfaceLocation = SteepestHitResult.ImpactPoint;
	CurrentClimbableSurfaceNormal = SteepestHitResult.ImpactNormal.GetSafeNormal();
}

bool UTHMovementComponent::CheckShouldStopClimbing() const
{
	if (ClimbableSurfacesTracedResults.IsEmpty()) return true;

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	
	if (DotResult >= GetWalkableFloorZ())
	{
		return true;
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

    FHitResult LedgeHit = TraceFromEyeHeight(100.f, 80.f);
    if (!LedgeHit.bBlockingHit)
    {
        const FVector WalkableSurfaceTraceStart = LedgeHit.TraceEnd;
        const FVector DownVector = -UpdatedComponent->GetUpVector();
        const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + DownVector * 150.f;
        FHitResult WalkableSurfaceHit = DoLineTraceSingleForObject(WalkableSurfaceTraceStart, WalkableSurfaceTraceEnd);

        if (WalkableSurfaceHit.bBlockingHit && FVector::DotProduct(WalkableSurfaceHit.ImpactNormal, FVector::UpVector) > GetWalkableFloorZ())
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

	const FVector N = CurrentClimbableSurfaceNormal.GetSafeNormal();
	const FVector WorldUp = FVector::UpVector;

	FVector ClimbUp = (WorldUp - FVector::DotProduct(WorldUp, N) * N).GetSafeNormal();

	if (ClimbUp.IsNearlyZero())
	{
		const FVector WorldRight = FVector::RightVector;
		ClimbUp = (WorldRight - FVector::DotProduct(WorldRight, N) * N).GetSafeNormal();
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromXZ(-N, ClimbUp).ToQuat();

	return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5.f);
}


void UTHMovementComponent::SnapMovementToClimbableSurfaces(float DeltaTime) const
{
	if (CurrentClimbableSurfaceNormal.IsNearlyZero()) return;

	const FVector N = CurrentClimbableSurfaceNormal.GetSafeNormal();
	const FVector P0 = CurrentClimbableSurfaceLocation;
	const FVector C = UpdatedComponent->GetComponentLocation(); 

	const float DesiredOffset = 20.f;

	const float signedDist = FVector::DotProduct(C - P0, N);
	const float delta = DesiredOffset - signedDist;

	const FQuat targetRot = GetClimbRotation(DeltaTime);
	if (FMath::Abs(delta) > KINDA_SMALL_NUMBER)
	{
		const FVector correction = N * delta;
		UpdatedComponent->MoveComponent(correction, targetRot, /*bSweep=*/true);
	}
	else
	{
		UpdatedComponent->MoveComponent(FVector::ZeroVector, targetRot, /*bSweep=*/true);
	}
}

void UTHMovementComponent::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay || !OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void UTHMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if (bEnableClimb)
	{
		if (CanStartClimbing())
		{
			StopMovementImmediately();
			StartClimbing();
			Multicast_PlayMontage(IdleToClimbMontage);
		}
		else if (CanClimbDownLedge())
		{
			StopMovementImmediately();
			StartClimbing();
			Multicast_PlayMontage(ClimbDownLedgeMontage);
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
		StopMovementImmediately();
		SetMotionWarpingTarget(FName("VaultStartPoint"), VaultStartPosition);
		SetMotionWarpingTarget(FName("VaultLandPoint"), VaultLandPosition);
		StartClimbing();
		Multicast_PlayMontage(VaultingMontage);
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

// void UTHMovementComponent::PlayClimbMontage(UAnimMontage* MontageToPlay) const
// {
// 	if (!MontageToPlay || !OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
// 	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
// }

void UTHMovementComponent::OnClimbMontageEnded(UAnimMontage* MontageToPlay, bool bInterrupted)
{
	if (MontageToPlay == IdleToClimbMontage || MontageToPlay == ClimbDownLedgeMontage)
	{
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
		const_cast<UTHMovementComponent*>(this)->Multicast_PlayMontage(HopUpMontage);
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
		const_cast<UTHMovementComponent*>(this)->Multicast_PlayMontage(HopDownMontage);
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