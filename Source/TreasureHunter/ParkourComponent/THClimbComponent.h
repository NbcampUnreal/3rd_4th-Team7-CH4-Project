#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "THClimbComponent.generated.h"

USTRUCT(BlueprintType)
struct FClimbTraceResult
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	bool bCanClimb = false;
	
	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize LedgeLocation;
	
	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize WallLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantizeNormal WallNormal;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TREASUREHUNTER_API UTHClimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTHClimbComponent();
	
	bool CheckClimbableSurface(FClimbTraceResult& OutResult) const;
	
	bool FindClimbableSurface(const FVector& DesiredDirection, FClimbTraceResult& OutResult) const;

	bool IsOnValidClimbSurface(const FVector& HitLocation, const FVector& WallNormal) const;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float ForwardTraceDistance = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace", meta = (DisplayName = "Climb Detection Capsule Radius"))
	float ClimbCapsuleTraceRadius = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace", meta = (DisplayName = "Climb Detection Capsule Half Height"))
	float ClimbCapsuleTraceHalfHeight = 70.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float LedgeTraceStartHeight = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float LedgeTraceDepth = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> ClimbableTraceChannel;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Climb")
	float MaxClimbSpeed = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace", meta = (DisplayName = "Max Climb Start Distance"))
	float MaxClimbStartDistance = 50.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace", meta = (DisplayName = "Min Climb Start Distance"))
	float MinClimbStartDistance = 35.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float ClimbDistanceTolerance = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace|Climb Movement")
	float SidewaysTraceOffset = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace|Climb Movement")
	float LedgeDetectionRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace|Edge Detection")
	float EdgeCheckOffset = 10.f;
	
private:
	bool TraceForWall(FHitResult& OutHit) const;
	bool TraceForLedge(const FHitResult& WallHit, FHitResult& OutLedgeHit) const;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
};