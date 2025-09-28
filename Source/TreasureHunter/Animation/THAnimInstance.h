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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GroundSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Direction;
        
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bShouldMove : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bIsFalling : 1;
        
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bIsCrouching : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bIsClimbing : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ClimbDirectionX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ClimbDirectionY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
	float ClimbingBlendSpeed = 10.f;
};
