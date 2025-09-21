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
	// [FIX] Mantle 시작 전 이동모드 저장 후 EndAbility에서 복원
	TEnumAsByte<EMovementMode> SavedMovementMode = MOVE_Walking; // 기본값
	uint8 SavedCustomMovementMode = 0; // [FIX] 커스텀 모드도 복원
	bool bSpringArmCollisionSaved = true; // [FIX] 스프링암 충돌 설정 원복
	FName SavedCapsuleProfile = NAME_None; // [FIX] 캡슐 콜리전 프로파일 원복용
};