#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THUseShotInkAbility.generated.h"


UCLASS()
class TREASUREHUNTER_API UTHUseShotInkAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffects")
	TSubclassOf<UGameplayEffect> InkShotEffectClass;


	bool CheckTag(UAbilitySystemComponent* TargetASC, FGameplayTag CheckGameplayTag);

};
