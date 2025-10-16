
#include "Item/ItemAbility/THUseJumpBoostAbility.h"
#include "AbilitySystemComponent.h"
#include "Item/ItemEffect/THJumpBoostEffect.h"
#include "Game/GameFlowTags.h"



void UTHUseJumpBoostAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

		FGameplayTag JumpBoostTag = TAG_Item_JumpBoost_Active;

		FGameplayEffectQuery Query;
		TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

		for (const FActiveGameplayEffectHandle& ActiveGEHandle : ActiveEffects)
		{
			const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(ActiveGEHandle);
			if (ActiveGE)
			{
				if (ActiveGE->Spec.DynamicGrantedTags.HasTag(JumpBoostTag))
				{
					ASC->RemoveActiveGameplayEffect(ActiveGEHandle);
					break;
				}
			}
		}

		FGameplayEffectSpecHandle JumpSpecHandle = MakeOutgoingGameplayEffectSpec(UTHJumpBoostEffect::StaticClass(), Level);
		if (JumpSpecHandle.IsValid() && JumpSpecHandle.Data.IsValid())
		{
			FGameplayEffectSpec* Spec = JumpSpecHandle.Data.Get();

			Spec->DynamicGrantedTags.AddTag(TAG_Item_JumpBoost_Active);

			ASC->ApplyGameplayEffectSpecToSelf(*Spec);
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
