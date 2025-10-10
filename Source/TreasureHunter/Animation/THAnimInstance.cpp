#include "Animation/THAnimInstance.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "ParkourComponent/THMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void UTHAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); 
	
	APawn* OwnerPawn = TryGetPawnOwner();
	if (IsValid(OwnerPawn))
	{
		PlayerCharacter = Cast<ATHPlayerCharacter>(OwnerPawn);
		if (IsValid(PlayerCharacter))
		{
			THMovementComponent = PlayerCharacter->GetTHMovementComponent();
		}
	}
}

void UTHAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
        
	if (IsValid(PlayerCharacter) && IsValid(THMovementComponent))
	{
		Velocity = THMovementComponent->Velocity;
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, PlayerCharacter->GetActorRotation());
		GroundSpeed = UKismetMathLibrary::VSizeXY(Velocity);
		
		float GroundAcceleration = UKismetMathLibrary::VSizeXY(THMovementComponent->GetCurrentAcceleration());
		bShouldMove = (GroundSpeed > 3.f) && (GroundAcceleration > 0.f);
		
		bIsFalling = THMovementComponent->IsFalling();
		bIsCrouching = THMovementComponent->IsCrouching();
		
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