#include "Animation/THAnimInstance.h"

#include "PlayerCharacter/THPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void UTHAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); 
	
	APawn* OwnerPawn = TryGetPawnOwner();
	if (IsValid(OwnerPawn) == true)
	{
		PlayerCharacter = Cast<ATHPlayerCharacter>(OwnerPawn);
		if (IsValid(PlayerCharacter) == true)
		{
			CharacterMovementComponent = PlayerCharacter->GetCharacterMovement();
		}
	}
}

void UTHAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
        
	if (IsValid(PlayerCharacter) == true && IsValid(CharacterMovementComponent) == true)
	{
		Velocity = CharacterMovementComponent->Velocity;
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, PlayerCharacter->GetActorRotation());
		GroundSpeed = UKismetMathLibrary::VSizeXY(Velocity);
		float GroundAcceleration = UKismetMathLibrary::VSizeXY(CharacterMovementComponent->GetCurrentAcceleration());
		bool bIsAccelerationNearlyZero = FMath::IsNearlyZero(GroundAcceleration);
		bShouldMove = (KINDA_SMALL_NUMBER < GroundSpeed) && (bIsAccelerationNearlyZero == false);
		bIsFalling = CharacterMovementComponent->IsFalling();
		bIsCrouching = CharacterMovementComponent->IsCrouching();
		
		FVector2D TargetDirection = FVector2D::ZeroVector;
		if (PlayerCharacter->bIsClimbing)
		{
			TargetDirection = PlayerCharacter->ClimbMovementDirection;
		}
		
		const FVector2D CurrentDirection = FVector2D(ClimbDirectionX, ClimbDirectionY);
		
		const FVector2D InterpedDirection = FMath::Vector2DInterpTo(CurrentDirection, TargetDirection, DeltaSeconds, ClimbingBlendSpeed);
		
		ClimbDirectionX = InterpedDirection.X;
		ClimbDirectionY = InterpedDirection.Y;
		
		bIsClimbing = PlayerCharacter->bIsClimbing;
	}
}

