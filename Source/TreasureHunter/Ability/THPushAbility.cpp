// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/THPushAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect/CoolDownEffect_Push.h"
#include "GameplayTagContainer.h"
#include "Game/GameFlowTags.h"
#include "Engine/OverlapResult.h"
#include "PlayerCharacter/THPlayerCharacter.h"


UTHPushAbility::UTHPushAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CooldownGameplayEffectClass = UCoolDownEffect_Push::StaticClass();

	AbilityTags.AddTag(TAG_Cooldown_Ability_Push);
	
	AbilityTags.AddTag(TAG_Ability_Push);

}

bool UTHPushAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                        const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

}

void UTHPushAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Push!")));
	
	if (PlayerCharacter)
	{
		TArray<FOverlapResult> OverlapResults;
	
		FVector Start = PlayerCharacter->GetActorLocation();
		FVector Forward = PlayerCharacter->GetActorForwardVector();
		FVector End = Start + (Forward * PushRange);

		TArray<FHitResult> HitResults;
		FCollisionShape Box = FCollisionShape::MakeBox(PushBoxSize);
		FRotator Rotation = PlayerCharacter->GetActorRotation();

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(PlayerCharacter);

		bool bHit = GetWorld()->OverlapMultiByObjectType(
			OverlapResults,
			Start,
			FQuat::Identity,
			ObjectQueryParams,
			Box,
			QueryParams
			);

		if (bHit)
		{
			for (FOverlapResult& Hit : OverlapResults)
			{
				if (ACharacter* Target = Cast<ACharacter>(Hit.GetActor()))
				{
					FVector TargetLocation = Target->GetActorLocation();
					FVector CurrentLocation = PlayerCharacter->GetActorLocation();
					FVector LaunchVector = (TargetLocation - CurrentLocation).GetSafeNormal();
			
					Target->LaunchCharacter((LaunchVector * PushForce) + FVector(0,0, 100.0f), true, true);
		
				}

			}
		}
	}

	if (PushMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, 
		NAME_None, 
		PushMontage, 
		1.2f
		);

		Task->OnCompleted.AddDynamic(this, &UTHPushAbility::OnMontageCompleted);

		Task->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}

}

void UTHPushAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTHPushAbility::OnMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}
