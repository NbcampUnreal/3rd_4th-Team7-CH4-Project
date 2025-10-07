// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "GA_THClimb.generated.h"

UCLASS()
class TREASUREHUNTER_API UGA_THClimb : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_THClimb();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UPROPERTY(EditDefaultsOnly) UAnimMontage* IdleToClimbMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ClimbDownMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TopOutMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* VaultMontage;
};
