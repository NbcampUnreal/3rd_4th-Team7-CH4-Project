#include "Ability/THSprintAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/GameplayAbility.h" 
#include "AttributeSet/THAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Game/GameFlowTags.h"
#include "GameFramework/Character.h"
#include "Player/THPlayerState.h"
#include "Abilities/GameplayAbilityTypes.h"

UTHSprintAbility::UTHSprintAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    FGameplayTagContainer AbilityTagsContainer;
    AbilityTagsContainer.AddTag(TAG_Ability_Sprint);
    SetAssetTags(AbilityTagsContainer);
    
    ActivationOwnedTags.AddTag(TAG_State_Movement_Sprinting);
    
    ActivationBlockedTags.AddTag(TAG_Status_Stamina_Empty);
    ActivationBlockedTags.AddTag(TAG_Status_State_Mantling);

    ActivationBlockedTags.AddTag(TAG_State_Cooldown_SprintAfterMantle);
}

bool UTHSprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    
    const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (Character)
    {
        const FVector HorizontalVelocity = FVector(Character->GetVelocity().X, Character->GetVelocity().Y, 0.0f);
        
        if (HorizontalVelocity.SizeSquared() > KINDA_SMALL_NUMBER)
        {
            return true;
        }
    }
    
    return false;
}

void UTHSprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    K2_AddGameplayCue(TAG_GameplayCue_Movement_Sprinting, MakeEffectContext(Handle, ActorInfo));
    
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured();
    
    ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_Effect_Stamina_Regen));
    
    if (HasAuthority(&ActivationInfo) && StaminaCostEffect)
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaCostEffect);
        if (SpecHandle.IsValid())
        {
            StaminaCostEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
    }
    
    UAbilityTask_WaitGameplayTagAdded* WaitStaminaEmptyTag = UAbilityTask_WaitGameplayTagAdded::WaitGameplayTagAdd(this, TAG_Status_Stamina_Empty);
    if (WaitStaminaEmptyTag) 
    {
        WaitStaminaEmptyTag->Added.AddDynamic(this, &UTHSprintAbility::K2_EndAbility);
        WaitStaminaEmptyTag->ReadyForActivation();
    }
    
    UAbilityTask_WaitGameplayEvent* WaitMovementStopEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_Movement_Stopped);
    if (WaitMovementStopEvent)
    {
        WaitMovementStopEvent->EventReceived.AddDynamic(this, &UTHSprintAbility::OnGameplayEvent);
        WaitMovementStopEvent->ReadyForActivation();
    }
}

void UTHSprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    K2_RemoveGameplayCue(TAG_GameplayCue_Movement_Sprinting);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured();
        const UTHAttributeSet* AttributeSet = ASC ? ASC->GetSet<UTHAttributeSet>() : nullptr;
        
        if (ASC && AttributeSet)
        {
            if (HasAuthority(&ActivationInfo) && StaminaCostEffectHandle.IsValid())
            {
                ASC->RemoveActiveGameplayEffect(StaminaCostEffectHandle);
            }
            
            if (HasAuthority(&ActivationInfo) && AttributeSet->GetStamina() < AttributeSet->GetMaxStamina())
            {
                AActor* OwnerActor = ASC->GetOwnerActor();
                if (const ATHPlayerState* PS = Cast<ATHPlayerState>(OwnerActor))
                {
                    TSubclassOf<UGameplayEffect> DefaultRegenEffect = PS->GetStaminaRegenEffectClass();
                    if (DefaultRegenEffect)
                    {
                        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DefaultRegenEffect, GetAbilityLevel());

                        (void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
                    }
                }
            }
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTHSprintAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTHSprintAbility::OnGameplayEvent(FGameplayEventData Payload)
{
    K2_CancelAbility();
}