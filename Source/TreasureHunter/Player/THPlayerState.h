#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"

#include "THPlayerState.generated.h"

class UGameplayEffect;
class UTHAttributeSet;
class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class TREASUREHUNTER_API ATHPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATHPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	UTHAttributeSet* GetAttributeSet() const;

	void InitializeAbilityActorInfo(APawn* NewPawn);

	void GiveStartupAbilities();

	UPROPERTY(ReplicatedUsing = OnRep_Nickname, BlueprintReadOnly)
	FString Nickname;

	UFUNCTION() void OnRep_Nickname();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	
private:
	bool bStartupAbilitiesGiven;
};
