#include "Item/ItemAbility/THRocketStunAbility.h"
#include "AbilitySystemComponent.h"

#include "Item/ItemEffect/THShotRocketEffect.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "Game/GameFlowTags.h"


UTHRocketStunAbility::UTHRocketStunAbility()
{
	ActivationBlockedTags.AddTag(TAG_Item_ImmunePotion_Active);
}


void UTHRocketStunAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		FGameplayTag StunTag = TAG_State_Debuff_Stun;
		bool bFoundEffect = false;

		FGameplayEffectQuery Query;
		TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

		for (const FActiveGameplayEffectHandle& EffectsHandle : ActiveEffects)
		{
			const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(EffectsHandle);
			if (ActiveGE)
			{
				if (ActiveGE->Spec.DynamicGrantedTags.HasTag(StunTag))
				{
					ASC->RemoveActiveGameplayEffect(EffectsHandle);
					break;
				}
			}
		}



		//------이펙트 추가 적용
		FGameplayEffectSpecHandle StunSpecHandle = MakeOutgoingGameplayEffectSpec(UTHShotRocketEffect::StaticClass(), Level);
		if (StunSpecHandle.IsValid() && StunSpecHandle.Data.IsValid())
		{
			FGameplayEffectSpec* Spec = StunSpecHandle.Data.Get();

			Spec->DynamicGrantedTags.AddTag(TAG_State_Debuff_Stun);

			ASC->ApplyGameplayEffectSpecToSelf(*Spec);

			bool bIsServer = (ASC->GetOwnerRole() == ROLE_Authority);

			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

