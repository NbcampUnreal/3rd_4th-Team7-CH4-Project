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

    // Gratned �±׿� ��ġ�Ǵ� ȿ�� ���� ���� 
    static void RemoveEffectsByGrantedTags(UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags);

    // Blocked �±� �����ϰ� ������ true
    static bool TargetBlocked(UAbilitySystemComponent* TargetASC, const FGameplayTagContainer& BlockedBy);

    // Ư�� �±� ȿ�� ����
    static void Cleanse(UAbilitySystemComponent* ASC, const FGameplayTagContainer& CleanseTags);

    // Duration ���� 
    static void SetCommonSetByCaller(struct FGameplayEffectSpec* Spec, const FTHItemData& Row);

    /**
     * ���� ���� + ���� GE ����
     * - Row.GrantedActiveTag�� ������ ���� �±��� ���� ȿ�� ����
     * - �� Spec�� SetByCaller/Duration ����
     * - �� Spec���� ���� �±׸� �ο�(���� ���� �� Ȯ���� Ž��/����)
     */
    static void ApplyUniqueEffect(UAbilitySystemComponent* ASC, const FTHItemData& Row);
};
