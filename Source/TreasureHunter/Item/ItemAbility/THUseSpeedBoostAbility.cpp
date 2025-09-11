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

	UE_LOG(LogTemp, Log, TEXT("Activating Speed Boost Ability"));
    if (HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Applying Speed Boost Gameplay Effect"));
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        int32 Level = GetAbilityLevel(Handle, ActorInfo);

        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UTHSpeedBoostEffect::StaticClass(), Level);

        if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully created Speed Boost Gameplay Effect Spec"));
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }

}