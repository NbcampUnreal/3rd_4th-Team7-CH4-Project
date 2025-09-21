#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "THMantleAbility.generated.h"

UCLASS()
class TREASUREHUNTER_API UTHMantleAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UTHMantleAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> StaminaCostEffect;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

private:
	// [FIX] Mantle ���� �� �̵���� ���� �� EndAbility���� ����
	TEnumAsByte<EMovementMode> SavedMovementMode = MOVE_Walking; // �⺻��
	uint8 SavedCustomMovementMode = 0; // [FIX] Ŀ���� ��嵵 ����
	bool bSpringArmCollisionSaved = true; // [FIX] �������� �浹 ���� ����
	FName SavedCapsuleProfile = NAME_None; // [FIX] ĸ�� �ݸ��� �������� ������
};