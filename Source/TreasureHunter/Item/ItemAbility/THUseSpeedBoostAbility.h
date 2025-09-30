#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THUseSpeedBoostAbility.generated.h"

UCLASS()
class TREASUREHUNTER_API UTHUseSpeedBoostAbility : public UGameplayAbility
{
	GENERATED_BODY()



protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

};
