#include "Item/ItemAbility/THUseImmunePotionAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Item/ItemEffect/THImmunePotionEffect.h"
#include "Game/GameFlowTags.h"





void UTHUseImmunePotionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

		FGameplayTag ImmunePotionTag = TAG_Item_ImmunePotion_Active;
		RemoveDebuff(ASC, ImmunePotionTag);
		


		FGameplayEffectSpecHandle ImmuneSpecHandle = MakeOutgoingGameplayEffectSpec(UTHImmunePotionEffect::StaticClass(), Level);
		if (ImmuneSpecHandle.IsValid() && ImmuneSpecHandle.Data.IsValid())
		{
			FGameplayEffectSpec* Spec = ImmuneSpecHandle.Data.Get();

			Spec->DynamicGrantedTags.AddTag(ImmunePotionTag);

			ASC->ApplyGameplayEffectSpecToSelf(*Spec);

			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);
		}

		RemoveDebuff(ASC, TAG_Item_SpeedSlow_Active);
		RemoveDebuff(ASC, TAG_Item_Stun_Active);
		RemoveDebuff(ASC, TAG_Item_Ink_Active);
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}





void UTHUseImmunePotionAbility::RemoveDebuff(UAbilitySystemComponent* ASC, FGameplayTag DelTag)
{
	FGameplayEffectQuery Query;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE)
		{
			if (ActiveGE->Spec.DynamicGrantedTags.HasTag(DelTag))
			{
				ASC->RemoveActiveGameplayEffect(Handle);
				break;
			}
		}
	}
}