// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THStompAbility.generated.h"

class UGameplayEffect;
/**
 * 
 */
UCLASS()
class TREASUREHUNTER_API UTHStompAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StompJumpForce = FVector(0.f, 0.f, 1000.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Headoffset = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stomp")
	TSubclassOf<UGameplayEffect> StunEffectClass;
	
protected:
	UTHStompAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

};
