// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THPushAbility.generated.h"

/**
 * 
 */
UCLASS()
class TREASUREHUNTER_API UTHPushAbility : public UGameplayAbility
{
	GENERATED_BODY()

private:
	UTHPushAbility();
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	UFUNCTION()
	void OnMontageCompleted();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PushBoxSize = FVector(100.f, 100.f, 100.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PushRange = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PushForce = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PushTimer = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* PushMontage;
};
