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




		//------태그제거용
		FGameplayTag SpeedBoostTag = FGameplayTag::RequestGameplayTag("Item.SpeedBoost.Active");
		bool bFoundEffect = false;

		FGameplayEffectQuery Query;
		TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

		for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
		{
			const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
			if (ActiveGE)
			{
				if (ActiveGE->Spec.DynamicGrantedTags.HasTag(SpeedBoostTag))
				{
					ASC->RemoveActiveGameplayEffect(Handle);
					break;
				}
			}
		}



		//------이펙트 추가 적용
		FGameplayEffectSpecHandle SpeedSpecHandle = MakeOutgoingGameplayEffectSpec(UTHSpeedBoostEffect::StaticClass(), Level);
		if (SpeedSpecHandle.IsValid() && SpeedSpecHandle.Data.IsValid())
		{
			FGameplayEffectSpec* Spec = SpeedSpecHandle.Data.Get();
						
			Spec->DynamicGrantedTags.AddTag(FGameplayTag::RequestGameplayTag("Item.SpeedBoost.Active"));
			
			ASC->ApplyGameplayEffectSpecToSelf(*Spec);
			
			bool bIsServer = (ASC->GetOwnerRole() == ROLE_Authority);
			
			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
