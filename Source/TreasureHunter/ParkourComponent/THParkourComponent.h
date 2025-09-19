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
	float MantleForwardOffset = 50.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float UpWarpTargetInwardOffset = 5.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle Settings")
	float FinalLandingHeightOffset = 88.f;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
};