// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayCue/THFootStepCue.h"

#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "PlayerCharacter/THPlayerCharacter.h"


void UTHFootStepCue::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType,
									   const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(MyTarget);

	if (UNiagaraComponent* FootStepComp = PlayerCharacter->GetFootStepComponent())
	{
		FootStepComp->Activate(true);
	}
	
	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),            
		FootStepSound,      
		Parameters.Location,    
		FRotator::ZeroRotator
	);
	
}
