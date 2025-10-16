#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THUseStaminaRecoveryAbility.generated.h"


UCLASS()
class TREASUREHUNTER_API UTHUseStaminaRecoveryAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
