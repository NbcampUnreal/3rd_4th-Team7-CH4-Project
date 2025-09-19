#include "Item/ItemEffect/THShotRocketEffect.h"
#include "AttributeSet/THAttributeSet.h"

UTHShotRocketEffect::UTHShotRocketEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(2.f);

    FGameplayModifierInfo WalkModifier;
    WalkModifier.Attribute = UTHAttributeSet::GetWalkSpeedAttribute();
    WalkModifier.ModifierOp = EGameplayModOp::Additive;
    WalkModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-2000.f));
    Modifiers.Add(WalkModifier);

    FGameplayModifierInfo SprintModifier;
    SprintModifier.Attribute = UTHAttributeSet::GetSprintSpeedAttribute();
    SprintModifier.ModifierOp = EGameplayModOp::Additive;
    SprintModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-4000.f));
    Modifiers.Add(SprintModifier);

}
