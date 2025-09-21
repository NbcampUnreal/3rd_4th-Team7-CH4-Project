#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "THAnimInstance.generated.h"

class ATHPlayerCharacter;
class UCharacterMovementComponent;


UCLASS()
class TREASUREHUNTER_API UTHAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ATHPlayerCharacter> PlayerCharacter;
        
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;

	// [FIX] 런타임 계산 값 → Transient/BlueprintReadOnly
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement")
	uint8 bShouldMove : 1;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement")
	uint8 bIsFalling : 1;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement")
	uint8 bIsCrouching : 1;
};
