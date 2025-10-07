// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "GA_THMantle.generated.h"

UCLASS()
class TREASUREHUNTER_API UGA_THMantle : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_THMantle();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnMantleMontageEnded();

	UPROPERTY(EditDefaultsOnly) UAnimMontage* MantleMontage;

	UPROPERTY(EditDefaultsOnly) bool bZeroGravityDuringMantle = true;
};
