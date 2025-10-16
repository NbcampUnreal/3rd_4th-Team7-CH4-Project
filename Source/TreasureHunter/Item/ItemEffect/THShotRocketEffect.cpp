#include "Item/ItemEffect/THShotRocketEffect.h"
#include "AttributeSet/THAttributeSet.h"

UTHShotRocketEffect::UTHShotRocketEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(2.f);

    //사실 이러면 문제가 생기는데.. 이 이펙트가 듀레이션 중에 밟기로 스턴이 걸리면 여기서 태그 해제하게되고.. 그러면......ㅎ;
    //밟기의 스턴이 갱신되지 않아 짧게 끝날 수도 있다.

    /*FGameplayModifierInfo WalkModifier;
    WalkModifier.Attribute = UTHAttributeSet::GetWalkSpeedAttribute();
    WalkModifier.ModifierOp = EGameplayModOp::Additive;
    WalkModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-2000.f));
    Modifiers.Add(WalkModifier);

    FGameplayModifierInfo SprintModifier;
    SprintModifier.Attribute = UTHAttributeSet::GetSprintSpeedAttribute();
    SprintModifier.ModifierOp = EGameplayModOp::Additive;
    SprintModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-4000.f));
    Modifiers.Add(SprintModifier);*/

}
