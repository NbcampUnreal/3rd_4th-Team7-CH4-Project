// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/THPushAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GAS/Effect/CoolDownEffect_Push.h"
#include "GameplayTagContainer.h"
#include "GAS/Tags/GameFlowTags.h"
#include "Engine/OverlapResult.h"
#include "Player/PlayerCharacter/THPlayerCharacter.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"


UTHPushAbility::UTHPushAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// [FIX] 서버 권위에서 판정/런치 수행
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	FGameplayTagContainer Tags = GetAssetTags();
	Tags.AddTag(TAG_Ability_Push);
	SetAssetTags(Tags);

	CooldownGameplayEffectClass = UCoolDownEffect_Push::StaticClass();
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
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Push!")));
	
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

					const FVector Dir = (Target->GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal();
					Target->LaunchCharacter((Dir * PushForce) + FVector(0,0, 100.0f), true, true);
		
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