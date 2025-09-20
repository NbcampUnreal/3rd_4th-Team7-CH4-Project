#pragma once

#include "Abilities/GameplayAbility.h"
#include "Item/Item_Ability/THItemAbilityBase.h"
#include "THItemGenericAbility.generated.h"

class UDataTable;
class UAbilitySystemComponent;
struct FTHItemData;

// 아이템 Row 기반 Generic 어빌리티로 어빌리티 전체 통합
UCLASS()
class TREASUREHUNTER_API UTHItemGenericAbility : public UTHItemAbilityBase
{
    GENERATED_BODY()

protected:
    UTHItemGenericAbility();

public:
    // 기본 데이터 테이블, 런타임에 매니저에서 가져옴 
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
    // SourceObject에서 RowName을 뽑아온다(UTHItemRowSource 우선, 없으면 UObject 이름)
    static FName ResolveRowNameFromSourceObject(const UObject* SourceObj);

    // 테이블에서 Row를 찾고 없으면 매니저에서 테이블 가져옴
    const FTHItemData* FindRow(FName RowName, const FGameplayAbilityActorInfo* ActorInfo);

    // 여기서, 아이템 Deploy하는 어빌리티 설정
    static void SpawnAtFeet(const FGameplayAbilityActorInfo* Info, TSubclassOf<AActor> SpawnClass, const FTHItemData& Row);
    static TArray<UAbilitySystemComponent*> ResolveTargets(const FTHItemData& Row, const FGameplayAbilityActorInfo* Info); // 월드에서 캐릭터 수집하여 고름

    // 헬퍼 함수들
    
};
