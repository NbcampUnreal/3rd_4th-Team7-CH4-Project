#pragma once

#include "Abilities/GameplayAbility.h"
#include "Item/Item_Ability/THItemAbilityBase.h"
#include "THItemGenericAbility.generated.h"

class UDataTable;
class UAbilitySystemComponent;
struct FTHItemData;

// ������ Row ��� Generic �����Ƽ�� �����Ƽ ��ü ����
UCLASS()
class TREASUREHUNTER_API UTHItemGenericAbility : public UTHItemAbilityBase
{
    GENERATED_BODY()

protected:
    UTHItemGenericAbility();

public:
    // �⺻ ������ ���̺�, ��Ÿ�ӿ� �Ŵ������� ������ 
    UPROPERTY(EditDefaultsOnly, Category = "Item")
    UDataTable* ItemDataTable = nullptr;

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEnd, bool bWasCancelled) override;

private:
    // SourceObject���� RowName�� �̾ƿ´�(UTHItemRowSource �켱, ������ UObject �̸�)
    static FName ResolveRowNameFromSourceObject(const UObject* SourceObj);

    // ���̺��� Row�� ã�� ������ �Ŵ������� ���̺� ������
    const FTHItemData* FindRow(FName RowName, const FGameplayAbilityActorInfo* ActorInfo);

    // ���⼭, ������ Deploy�ϴ� �����Ƽ ����
    static void SpawnAtFeet(const FGameplayAbilityActorInfo* Info, TSubclassOf<AActor> SpawnClass, const FTHItemData& Row);
    static TArray<UAbilitySystemComponent*> ResolveTargets(const FTHItemData& Row, const FGameplayAbilityActorInfo* Info); // ���忡�� ĳ���� �����Ͽ� ��

    // ���� �Լ���
    
};
