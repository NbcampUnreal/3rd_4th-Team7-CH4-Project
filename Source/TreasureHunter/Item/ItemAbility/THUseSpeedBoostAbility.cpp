#include "THUseSpeedBoostAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Item/ItemEffect/THSpeedBoostEffect.h"


void UTHUseSpeedBoostAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{	
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
	}
    	
    if (HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        int32 Level = GetAbilityLevel(Handle, ActorInfo);

        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UTHSpeedBoostEffect::StaticClass(), Level);

        if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }


    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}