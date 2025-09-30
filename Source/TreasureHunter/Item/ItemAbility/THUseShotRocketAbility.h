#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THUseShotRocketAbility.generated.h"


UCLASS()
class TREASUREHUNTER_API UTHUseShotRocketAbility : public UGameplayAbility
{
	GENERATED_BODY()




protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;


	

	bool CheckTag(UAbilitySystemComponent* TargetASC, FGameplayTag CheckGameplayTag);
};
