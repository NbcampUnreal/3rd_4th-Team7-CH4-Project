// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/THStompAbility.h"

#include "Components/CapsuleComponent.h"
#include "Game/GameFlowTags.h"
#include "PlayerCharacter/THPlayerCharacter.h"

UTHStompAbility::UTHStompAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = TAG_Event_Hit_Falling;
	AbilityTriggers.Add(TriggerData);
}

bool UTHStompAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UTHStompAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (!PlayerCharacter || !TriggerEventData)
	{
		return;
	}

	if (TriggerEventData->TargetData.Num() > 0)
	{
		const FHitResult* HitResult = TriggerEventData->TargetData.Get(0)->GetHitResult();

		if (ACharacter* OtherCharacter = Cast<ACharacter>(HitResult->GetActor()))
		{
			float ImpactZ = HitResult->ImpactPoint.Z;
			float CharacterZ = OtherCharacter->GetActorLocation().Z
				+ OtherCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
				- Headoffset;

			FVector ImpactPoint = HitResult->ImpactPoint;
			/*DrawDebugSphere(
				GetWorld(),
				ImpactPoint,           // 위치
				20.0f,                 // 반지름
				12,                    // 세그먼트 수
				FColor::Red,           // 색상
				false,                 // 영구 표시 여부
				2.0f                   // 지속 시간 (초)
			);

			DrawDebugLine(
				GetWorld(),
				ImpactPoint,
				ImpactPoint + HitResult->ImpactNormal * 100.0f,
				FColor::Green,
				false,
				2.0f,
				0,
				2.0f
			);*/
			
			if (ImpactZ > CharacterZ)
			{
				PlayerCharacter->LaunchCharacter(StompJumpForce,false, true);

				if (UAbilitySystemComponent* TargetASC = Cast<ATHPlayerCharacter>(OtherCharacter)->GetAbilitySystemComponent())
				{
					if (StunEffectClass)
					{
						FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
						EffectContext.AddInstigator(PlayerCharacter, PlayerCharacter);
						
						FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(StunEffectClass, 1, EffectContext);
						if (SpecHandle.IsValid())
						{
							TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
						}
					}
				}
			}
		}
		
	}


	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTHStompAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
