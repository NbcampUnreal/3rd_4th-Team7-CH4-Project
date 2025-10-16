#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THRocketStunAbility.generated.h"


UCLASS()
class TREASUREHUNTER_API UTHRocketStunAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UTHRocketStunAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;




	bool CheckTag(UAbilitySystemComponent* TargetASC, FGameplayTag CheckGameplayTag);
};
