#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

#include "THPlayerState.generated.h"

class UGameplayEffect;
class UTHAttributeSet;

UCLASS()
class TREASUREHUNTER_API ATHPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ATHPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UTHAttributeSet* GetAttributeSet() const;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UTHAttributeSet> AttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> StaminaRegenEffect;
	
	
};
