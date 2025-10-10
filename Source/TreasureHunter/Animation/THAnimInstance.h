#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "THAnimInstance.generated.h"

class ATHPlayerCharacter;
class UTHMovementComponent;

UCLASS()
class TREASUREHUNTER_API UTHAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<ATHPlayerCharacter> PlayerCharacter;
        
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UTHMovementComponent> THMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction;
        
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	uint8 bShouldMove : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	uint8 bIsFalling : 1;
        
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	uint8 bIsCrouching : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Climb")
	bool bIsClimbing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Climb")
	float ClimbDirectionX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Climb")
	float ClimbDirectionY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Climb")
	float ClimbingBlendSpeed = 10.f;
};