#include "Item/ItemAbility/THUseShotRocketAbility.h"

#include "AbilitySystemComponent.h"

#include "Item/ItemEffect/THShotRocketEffect.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter/THPlayerCharacter.h"




void UTHUseShotRocketAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		ATHPlayerCharacter* MyCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());
		if (!MyCharacter)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
				
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATHPlayerCharacter::StaticClass(), FoundActors);

		for (AActor* FoundActor : FoundActors)
		{
			if (FoundActor && FoundActor != MyCharacter)
			{
				ATHPlayerCharacter* TargetCharacter = Cast<ATHPlayerCharacter>(FoundActor);
				if (TargetCharacter)
				{
					UAbilitySystemComponent* TargetASC = TargetCharacter->GetAbilitySystemComponent();
					if (TargetASC)
					{
						bool bBlockEffect = CheckTag(TargetASC, FGameplayTag::RequestGameplayTag("Item.ImmunePotion.Active"));
						if (bBlockEffect) return;


						//------태그제거용
						FGameplayTag SpeedBoostTag = FGameplayTag::RequestGameplayTag("Item.Stun.Active");
						
						FGameplayEffectQuery Query;
						TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

						for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
						{
							const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
							if (ActiveGE)
							{
								if (ActiveGE->Spec.DynamicGrantedTags.HasTag(SpeedBoostTag))
								{
									TargetASC->RemoveActiveGameplayEffect(Handle);
									break;
								}
							}
						}



						//------이펙트 추가 적용
						int32 Level = GetAbilityLevel(Handle, ActorInfo);

						FGameplayEffectSpecHandle SpeedSpecHandle = MakeOutgoingGameplayEffectSpec(UTHShotRocketEffect::StaticClass(), Level);
						if (SpeedSpecHandle.IsValid() && SpeedSpecHandle.Data.IsValid())
						{
							FGameplayEffectSpec* Spec = SpeedSpecHandle.Data.Get();

							Spec->DynamicGrantedTags.AddTag(SpeedBoostTag);

							TargetASC->ApplyGameplayEffectSpecToSelf(*Spec);

							FGameplayTagContainer OwnedTags;
							TargetASC->GetOwnedGameplayTags(OwnedTags);
						}




					}

					
					break;
				}
			}
		}


		
		
		
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}



bool UTHUseShotRocketAbility::CheckTag(UAbilitySystemComponent* TargetASC, FGameplayTag CheckGameplayTag)
{
	FGameplayEffectQuery Query;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE)
		{
			if (ActiveGE->Spec.DynamicGrantedTags.HasTag(CheckGameplayTag))
			{
				return true;
				break;
			}
		}
	}
	return false;
}

