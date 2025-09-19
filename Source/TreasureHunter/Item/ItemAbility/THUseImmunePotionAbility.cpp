#include "Item/ItemAbility/THUseImmunePotionAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Item/ItemEffect/THImmunePotionEffect.h"





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

		FGameplayTag ImmunePotionTag = FGameplayTag::RequestGameplayTag("Item.ImmunePotion.Active");
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

		RemoveDebuff(ASC, FGameplayTag::RequestGameplayTag("Item.SpeedSlow.Active"));
		RemoveDebuff(ASC, FGameplayTag::RequestGameplayTag("Item.Stun.Active"));
		RemoveDebuff(ASC, FGameplayTag::RequestGameplayTag("Item.Ink.Active"));
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