#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ParkourComponent/THClimbComponent.h"
#include "THClimbAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class TREASUREHUNTER_API UTHClimbAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UTHClimbAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Montage")
    TObjectPtr<UAnimMontage> LedgeGrabMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Montage")
    TSubclassOf<UGameplayEffect> StaminaCostEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Montage")
    TSubclassOf<UGameplayEffect> StaminaRegenEffect;
    
    UPROPERTY(EditDefaultsOnly, Category = "Climb")
    float WallOffset = 0.f;

    UPROPERTY(EditDefaultsOnly, Category = "Climb")
    float LedgeGrabVerticalOffset = 95.f;

    UPROPERTY(EditDefaultsOnly, Category = "Climb", meta = (DisplayName = "Wall Inset"))
    float WallInset = 50.f; 

    void SetupMotionWarping(const FClimbTraceResult& InClimbTraceResult);
    void PlayLedgeGrabMontage();
    
private:
    UFUNCTION()
    void OnLedgeGrabMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
    
    FClimbTraceResult ClimbTraceResult;
};