#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THSpeedSlowAbility.generated.h"


UCLASS()
class TREASUREHUNTER_API UTHSpeedSlowAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UTHSpeedSlowAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

};
