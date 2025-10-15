// Fill out your copyright notice in the Description page of Project Settings.


#include "THPushAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/OverlapResult.h"
#include "Game/GameFlowTags.h"
#include "GameplayEffect/CoolDownEffect_Push.h"
#include "PlayerCharacter/THPlayerCharacter.h"


UTHPushAbility::UTHPushAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	
	FGameplayTagContainer Tags = GetAssetTags();
	Tags.AddTag(TAG_Ability_Push);
	SetAssetTags(Tags);
	
	AbilityTags.AddTag(TAG_Ability_Push);

	//CooldownGameplayEffectClass = UCoolDownEffect_Push::StaticClass();
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

	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());

	if (PlayerCharacter->HasAuthority())
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
		
		if (PlayerCharacter)
		{
			FVector Start = PlayerCharacter->GetActorLocation();
			FVector Forward = PlayerCharacter->GetActorForwardVector();
			//FVector End = Start + (Forward * PushRange);

			const FVector Center = Start + Forward * (PushRange * 0.5f);
			TArray<FOverlapResult> OverlapResults;

			//TArray<FHitResult> HitResults;
			//FCollisionShape Box = FCollisionShape::MakeBox(PushBoxSize);
			FRotator Rotation = PlayerCharacter->GetActorRotation();

			FCollisionObjectQueryParams ObjectQueryParams;
			ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PushOverlap), false, PlayerCharacter);
			//FCollisionQueryParams QueryParams;
			//QueryParams.AddIgnoredActor(PlayerCharacter);

			bool bHit = GetWorld()->OverlapMultiByObjectType(
				OverlapResults,
				Center,
				Rotation.Quaternion(), // [FIX] 회전 적용
				ObjectQueryParams,
				FCollisionShape::MakeBox(FVector(PushRange * 0.5f, PushBoxSize.Y, PushBoxSize.Z)),
				QueryParams
				);

			if (bHit)
			{
				for (FOverlapResult& Hit : OverlapResults)
				{
					if (ACharacter* Target = Cast<ACharacter>(Hit.GetActor()))
					{
						if (Target == PlayerCharacter) continue;

						FVector Dir = (Target->GetActorLocation() - PlayerCharacter->GetActorLocation());
						Dir.Z = 0.0f;
						Target->LaunchCharacter((Dir.GetSafeNormal() * PushForce) + ZForce, true, true);
		
					}

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

		if (!Task) return;

		Task->OnCompleted.AddDynamic(this, &UTHPushAbility::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UTHPushAbility::OnMontageInterrupted); // [FIX]
		Task->OnCancelled.AddDynamic(this, &UTHPushAbility::OnMontageInterrupted);   // [FIX]

		Task->ReadyForActivation();
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
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

void UTHPushAbility::OnMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}