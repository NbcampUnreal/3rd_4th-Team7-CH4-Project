#include "Item/ItemAbility/THSpeedSlowAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Item/ItemEffect/THSpeedSlowEffect.h"
#include "Game/GameFlowTags.h"




UTHSpeedSlowAbility::UTHSpeedSlowAbility()
{
	ActivationBlockedTags.AddTag(TAG_Item_ImmunePotion_Active);
}



void UTHSpeedSlowAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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


		//------태그제거용		
		FGameplayTag SpeedSlowTag = TAG_Item_SpeedSlow_Active;
		
		FGameplayEffectQuery Query;
		TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);
		
		for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
		{
			const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
			if (ActiveGE)
			{
				if (ActiveGE->Spec.DynamicGrantedTags.HasTag(SpeedSlowTag))
				{
					ASC->RemoveActiveGameplayEffect(Handle);
				}
			}
		}

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			UTHSpeedSlowEffect::StaticClass(), Level, ASC->MakeEffectContext());

		if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
		{
			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
			Spec->DynamicGrantedTags.AddTag(SpeedSlowTag);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec);

			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}


