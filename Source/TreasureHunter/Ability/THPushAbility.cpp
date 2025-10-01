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
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

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
	
    if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo)) 
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
          
          const FVector Center = Start + Forward * (PushRange * 0.5f);
          TArray<FOverlapResult> OverlapResults;
          FRotator Rotation = PlayerCharacter->GetActorRotation();

          FCollisionObjectQueryParams ObjectQueryParams;
          ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

          FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PushOverlap), false, PlayerCharacter);

          bool bHit = GetWorld()->OverlapMultiByObjectType(
             OverlapResults,
             Center,
             Rotation.Quaternion(), 
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
    }
    
    if (PushMontage)
    {
       UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
       this, 
       NAME_None, 
       PushMontage, 
       1.2f
       );

       if (!Task) 
       {
          EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
          return; 
       }

       Task->OnCompleted.AddDynamic(this, &UTHPushAbility::OnMontageCompleted);
       Task->OnInterrupted.AddDynamic(this, &UTHPushAbility::OnMontageInterrupted);
       Task->OnCancelled.AddDynamic(this, &UTHPushAbility::OnMontageInterrupted);

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