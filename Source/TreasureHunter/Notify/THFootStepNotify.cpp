// Fill out your copyright notice in the Description page of Project Settings.


#include "Notify/THFootStepNotify.h"

#include "AbilitySystemGlobals.h"
#include "NiagaraComponent.h"
#include "Game/GameFlowTags.h"
#include "PlayerCharacter/THPlayerCharacter.h"


void UTHFootStepNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(MeshComp->GetOwner());
	
	if (PlayerCharacter)
	{
		FVector Location = MeshComp->GetSocketLocation(FootSocket);

		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MeshComp->GetOwner()))
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = Location;

			ASC->ExecuteGameplayCue(TAG_GamePlayCue_FootStep, CueParams);
			
		}
	}
}
