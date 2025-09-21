#include "THMantleNotify.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTHMantleNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
		{
			CMC->SetMovementMode(MOVE_Walking);
		}
	}
}
