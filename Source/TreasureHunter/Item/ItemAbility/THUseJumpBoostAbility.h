#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THUseJumpBoostAbility.generated.h"

UCLASS()
class TREASUREHUNTER_API UTHUseJumpBoostAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

