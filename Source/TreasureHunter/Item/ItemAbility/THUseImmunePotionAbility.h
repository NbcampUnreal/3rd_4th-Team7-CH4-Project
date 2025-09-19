#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THUseImmunePotionAbility.generated.h"


UCLASS()
class TREASUREHUNTER_API UTHUseImmunePotionAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	void RemoveDebuff(UAbilitySystemComponent* ASC, FGameplayTag DelTag);

};
