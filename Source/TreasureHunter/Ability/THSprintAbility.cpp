#include "Ability/THSprintAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AttributeSet/THAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Game/GameFlowTags.h"

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
}

bool UTHSprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    
    return true;
}

void UTHSprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    
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
            
            if (HasAuthority(&ActivationInfo) && StaminaRegenEffect && AttributeSet->GetStamina() < AttributeSet->GetMaxStamina())
            {
                const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaRegenEffect);
                
                if (SpecHandle.IsValid())
                {
                    static_cast<void>(ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle));
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