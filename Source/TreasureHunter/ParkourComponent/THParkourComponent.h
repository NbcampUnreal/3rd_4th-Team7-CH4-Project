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
	FVector LedgeLocation;
	
	UPROPERTY(BlueprintReadOnly)
	FVector TargetLocation;
	
	UPROPERTY(BlueprintReadOnly)
	FRotator TargetRotation;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UPrimitiveComponent> TargetComponent;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mantle")
	TObjectPtr<UAnimMontage> MantlingMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mantle")
	float MantleTraceDistance = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mantle")
	float MinMantleHeight = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mantle")
	float MaxMantleHeight = 500.f;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;

	virtual void BeginPlay() override;
};