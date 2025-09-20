#include "THItemAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Item/Item_Data/THItemData.h"

UAbilitySystemComponent* UTHItemAbilityBase::GetASC(const FGameplayAbilityActorInfo* Info)
{
    return (Info && Info->AbilitySystemComponent.IsValid()) ? Info->AbilitySystemComponent.Get() : nullptr;
}

void UTHItemAbilityBase::RemoveEffectsByGrantedTags(UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags)
{
    if (!ASC || Tags.IsEmpty()) return;

    // Owning == Granted
    FGameplayEffectQuery Q = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(Tags);
    ASC->RemoveActiveEffects(Q);

    // ASC->RemoveActiveEffectsWithGrantedTags(Tags);
}

bool UTHItemAbilityBase::TargetBlocked(UAbilitySystemComponent* TargetASC, const FGameplayTagContainer& BlockedBy)
{
    if (!TargetASC || BlockedBy.IsEmpty()) return false;

    FGameplayTagContainer Owned;
    TargetASC->GetOwnedGameplayTags(Owned);
    return Owned.HasAny(BlockedBy);
}

void UTHItemAbilityBase::Cleanse(UAbilitySystemComponent* ASC, const FGameplayTagContainer& CleanseTags)
{
    if (!ASC || CleanseTags.IsEmpty()) return;
    RemoveEffectsByGrantedTags(ASC, CleanseTags);
}

void UTHItemAbilityBase::SetCommonSetByCaller(FGameplayEffectSpec* Spec, const FTHItemData& Row)
{
    if (!Spec) return;

    // Data.WalkDelta / Data.SprintDelta / Data.JumpDelta / Data.StaminaDelta
    // 아이템들의 효과를 데이터 테이블 델타(변화량) 값 기준으로 변경 -> 테이블로 모두 관리 
    Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.WalkDelta")), Row.WalkDelta);
    Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.SprintDelta")), Row.SprintDelta);
    Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.JumpDelta")), Row.JumpDelta);
    Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.StaminaDelta")), Row.StaminaDelta);

    if (Row.DurationSec > 0.f)
    {
        Spec->SetDuration(Row.DurationSec, true);
    }
}

void UTHItemAbilityBase::ApplyUniqueEffect(UAbilitySystemComponent* ASC, const FTHItemData& Row)
{
    if (!ASC || !Row.GameplayEffectClass) return;

    if (Row.GrantedActiveTag.IsValid()) // Granted Tag 제거 
    {
        FGameplayTagContainer C;
        C.AddTag(Row.GrantedActiveTag);
        RemoveEffectsByGrantedTags(ASC, C);
    }

    FGameplayEffectSpecHandle H = ASC->MakeOutgoingSpec(Row.GameplayEffectClass, 1, ASC->MakeEffectContext()); // Spec 생성
    if (!H.IsValid()) return;

    FGameplayEffectSpec* Spec = H.Data.Get();
    SetCommonSetByCaller(Spec, Row);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec);
}

