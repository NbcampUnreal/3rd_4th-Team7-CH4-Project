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

	UPROPERTY(BlueprintReadOnly) FVector LedgeLocation;
	
	UPROPERTY(BlueprintReadOnly) FVector LedgeNormal;
	
	UPROPERTY(BlueprintReadOnly) FVector TargetLocation;
	
	UPROPERTY(BlueprintReadOnly) FRotator TargetRotation;
	
	UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<UPrimitiveComponent> TargetComponent;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TREASUREHUNTER_API UTHParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTHParkourComponent();

	bool CheckMantle(FMantleInfo& OutMantleInfo) const;

	UAnimMontage* GetMantlingMontage() const { return MantlingMontage; }
	
	float GetMantleTraceDistance() const { return MantleTraceDistance; }
	
	float GetMinMantleHeight() const { return MinMantleHeight; }
	
	float GetMaxMantleHeight() const { return MaxMantleHeight; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mantle")
	TObjectPtr<UAnimMontage> MantlingMontage;

private:
	UPROPERTY(VisibleAnywhere, Category = "Mantle")
	float MantleTraceDistance;

	UPROPERTY(VisibleAnywhere, Category = "Mantle")
	float MinMantleHeight;

	UPROPERTY(VisibleAnywhere, Category = "Mantle")
	float MaxMantleHeight;

	UPROPERTY(Transient) TObjectPtr<ACharacter> OwnerCharacter;
	
	UPROPERTY(Transient) TObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
};
