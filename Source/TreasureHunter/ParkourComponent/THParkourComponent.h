#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "THParkourComponent.generated.h"

class UAnimMontage;
class ACharacter;
class UCharacterMovementComponent;

USTRUCT(BlueprintType)
struct FMantleInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FVector LedgeLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly)
	FTransform UpWarpTarget = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly)
	FTransform ForwardWarpTarget = FTransform::Identity;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UPrimitiveComponent> TargetComponent = nullptr;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TREASUREHUNTER_API UTHParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTHParkourComponent();

	bool CheckMantle(FMantleInfo& OutMantleInfo) const;
	
	UAnimMontage* GetMantlingMontage() const { return MantlingMontage; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	TObjectPtr<UAnimMontage> MantlingMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float MantleReachDistance = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float MinMantleHeight = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float MaxMantleHeight = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float MantleForwardOffset = 25.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float MantleUpZOffset = 3.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float FinalLandingHeightOffset = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float MantleHandPlacementOffset = 13.f;

private:
	bool TraceForWall(FHitResult& OutFrontHit) const;
	bool TraceForLedge(const FHitResult& FrontHit, FHitResult& OutSurfaceHit) const;
	bool IsLandingSpaceClear(const FVector& LandingCapsuleCenter) const;
	void CalculateWarpTargets(const FHitResult& FrontHit, const FHitResult& SurfaceHit, const FVector& FinalLandingLocation, FMantleInfo& OutMantleInfo) const;
	
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
};