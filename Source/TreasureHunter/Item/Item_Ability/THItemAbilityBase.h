#pragma once

#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "THItemAbilityBase.generated.h"

struct FTHItemData;
class UAbilitySystemComponent;

UCLASS(Abstract)
class TREASUREHUNTER_API UTHItemAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()

protected:
    static UAbilitySystemComponent* GetASC(const FGameplayAbilityActorInfo* Info);

    // Gratned 태그와 매치되는 효과 전부 제거 
    static void RemoveEffectsByGrantedTags(UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags);

    // Blocked 태그 보유하고 있으면 true
    static bool TargetBlocked(UAbilitySystemComponent* TargetASC, const FGameplayTagContainer& BlockedBy);

    // 특정 태그 효과 제거
    static void Cleanse(UAbilitySystemComponent* ASC, const FGameplayTagContainer& CleanseTags);

    // Duration 넣음 
    static void SetCommonSetByCaller(struct FGameplayEffectSpec* Spec, const FTHItemData& Row);

    /**
     * 유일 적용 + 공용 GE 적용
     * - Row.GrantedActiveTag가 있으면 동일 태그의 기존 효과 제거
     * - 새 Spec에 SetByCaller/Duration 주입
     * - 새 Spec에도 동일 태그를 부여(다음 갱신 때 확실히 탐지/제거)
     */
    static void ApplyUniqueEffect(UAbilitySystemComponent* ASC, const FTHItemData& Row);
};
