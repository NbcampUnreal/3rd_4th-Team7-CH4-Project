// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "THCharacterMovementComponent.generated.h"

DECLARE_DELEGATE(FOnEnterClimbState)
DECLARE_DELEGATE(FOnExitClimbState)

class UAnimMontage;
class UAnimInstance;
class ATHPlayerCharacter;

UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		MOVE_Climb UMETA(DisplayName = "Climb Mode")
	};
}

UCLASS()
class TREASUREHUNTER_API UTHCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	FOnEnterClimbState OnEnterClimbStateDelegate;
	FOnExitClimbState OnExitClimbStateDelegate;

protected:
#pragma region Overriden Functions
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;
#pragma endregion

private:
#pragma region ClimbTraces
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool bDrawPersistentShapes = false) const;
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool bDrawPersistentShapes = false) const;
#pragma endregion

#pragma region ClimbCore
	TArray<FHitResult> GetClimbableSurfaces();
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f, bool bShowDebugShape = false, bool bDrawPersistantShapes = false);

public:
	bool CanStartClimbing();
	bool CanClimbDownLedge();

private:
	void StartClimbing();
	void StopClimbing();
	void PhysClimb(float deltaTime, int32 Iterations);
	void ProcessClimbableSurfaceInfo();
	bool ShouldStopClimbing();
	bool CheckHasReachedFloor();
	FQuat GetClimbRotation(float DeltaTime);
	void SnapMovementToClimbableSurfaces(float DeltaTime);
	bool CheckHasReachedLedge();
	void TryStartVaulting();
	bool CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition);
	void PlayClimbMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition);

private:
	void HandleHopUp();
	bool CheckCanHopUp(FVector& OutHopUpTargetPosition);
	void HandleHopDown();
	bool CheckCanHopDown(FVector& OutHopDownTargetPosition);
#pragma endregion

#pragma region ClimbCoreVariables
	TArray<FHitResult> ClimbableSurfacesTracedResults;
	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;

	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	UPROPERTY()
	ATHPlayerCharacter* OwningPlayerCharacter;
#pragma endregion

#pragma region ClimbBPVariables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"));
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbableSurfaceTraceTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"));
	float ClimbCapsuleTraceRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"));
	float ClimbCapsuleTraceHalfHeight = 72;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbAcceleration = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbDownWalkableSurfaceTraceOffset = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbDownLedgeTraceOffset = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToClimbMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbToTopMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbDownLedgeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* VaultMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HopUpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HopDownMontage;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Mantle")
	float MaxMantleSurfaceAngle = 65.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Mantle")
	float MantleForwardDistance = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Mantle")
	float MantleTopDownCheck = 150.f;

public:
	bool CanStartMantle(FVector& OutStart, FVector& OutEnd) const;
#pragma endregion

public:
	void ToggleClimbing(bool bAttemptClimbing);
	void RequestHopping();
	bool IsClimbing() const;
	FORCEINLINE FVector GetClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; }
	FVector GetUnrotatedClimbVelocity() const;

	FORCEINLINE UAnimMontage* GetIdleToClimbMontage() const { return IdleToClimbMontage; }
	FORCEINLINE UAnimMontage* GetClimbDownLedgeMontage() const { return ClimbDownLedgeMontage; }
};
