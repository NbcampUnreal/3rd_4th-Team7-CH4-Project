#include "Item/ItemEffect/THSpeedBoostEffect.h"
#include "AttributeSet/THAttributeSet.h"

UTHSpeedBoostEffect::UTHSpeedBoostEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(30.f);

    FGameplayModifierInfo WalkModifier;
    WalkModifier.Attribute = UTHAttributeSet::GetWalkSpeedAttribute();
    WalkModifier.ModifierOp = EGameplayModOp::Additive;
    WalkModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(200.f));
    Modifiers.Add(WalkModifier);

    FGameplayModifierInfo SprintModifier;
    SprintModifier.Attribute = UTHAttributeSet::GetSprintSpeedAttribute();
    SprintModifier.ModifierOp = EGameplayModOp::Additive;
    SprintModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(500.f));
    Modifiers.Add(SprintModifier);

}
