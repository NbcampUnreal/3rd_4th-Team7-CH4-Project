#include "ParkourComponent/THParkourComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UTHParkourComponent::UTHParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MantleTraceDistance = 150.f;
	MinMantleHeight = 50.f;
	MaxMantleHeight = 200.f;

}

void UTHParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if(IsValid(OwnerCharacter) == true)
	{
		OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}