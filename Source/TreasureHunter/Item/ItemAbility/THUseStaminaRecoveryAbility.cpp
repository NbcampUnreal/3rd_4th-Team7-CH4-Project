#include "Item/ItemAbility/THUseStaminaRecoveryAbility.h"
#include "AbilitySystemComponent.h"
#include "Item/ItemEffect/THStaminaRecoveryEffect.h"




void UTHUseStaminaRecoveryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
				
		FGameplayEffectSpecHandle StaminaSpecHandle = MakeOutgoingGameplayEffectSpec(UTHStaminaRecoveryEffect::StaticClass(), Level);
		if (StaminaSpecHandle.IsValid() && StaminaSpecHandle.Data.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*StaminaSpecHandle.Data.Get());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}