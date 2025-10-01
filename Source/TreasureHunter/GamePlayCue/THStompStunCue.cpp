
#include "GamePlayCue/THStompStunCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"   
#include "NiagaraCommon.h"            
#include "NiagaraComponent.h"
#include "NiagaraSystem.h" 
#include "GameFramework/Character.h"
#include "PlayerCharacter/THPlayerCharacter.h"


void UTHStompStunCue::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType,
                                        const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	if (!ShakeClass) return;
	
	if (APlayerController* PC = Cast<APlayerController>(Parameters.Instigator->GetInstigatorController()))
	{
		PC->ClientStartCameraShake(ShakeClass, ShakeScale);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CameraShake"));
	}
	
	
	if (StompSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),            
		StompSound,      
		Parameters.Location,    
		FRotator::ZeroRotator
		);
	}

	if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(MyTarget))
	{
		PlayerCharacter->PlayAnimMontage(StunAnimation, 1.0f);
	}
}

