#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "THUseSpawnObjectAbility.generated.h"



UCLASS()
class TREASUREHUNTER_API UTHUseSpawnObjectAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;


	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object")
	TSubclassOf<AActor> ObjectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object")
	int32 MaxCreateObjectHeight = 300;
};
