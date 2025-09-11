#include "Notify/THMantleNotify.h"
#include "GameFramework/Character.h"
#include "ParkourComponent/THParkourComponent.h"

void UTHMantleNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		if (UTHParkourComponent* ParkourComponent = Character->FindComponentByClass<UTHParkourComponent>())
		{
			FMantleInfo MantleInfo;

			if (ParkourComponent->CheckMantle(MantleInfo))
			{
				Character->SetActorLocationAndRotation(MantleInfo.TargetLocation, MantleInfo.TargetRotation);
			}
		}
	}
}