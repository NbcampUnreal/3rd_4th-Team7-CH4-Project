#include "Item/ItemEffect/THSpeedSlowEffect.h"
#include "AttributeSet/THAttributeSet.h"

UTHSpeedSlowEffect::UTHSpeedSlowEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(5.f);

    FGameplayModifierInfo WalkModifier;
    WalkModifier.Attribute = UTHAttributeSet::GetWalkSpeedAttribute();
    WalkModifier.ModifierOp = EGameplayModOp::Additive;
    WalkModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-150.f));
    Modifiers.Add(WalkModifier);

    FGameplayModifierInfo SprintModifier;
    SprintModifier.Attribute = UTHAttributeSet::GetSprintSpeedAttribute();
    SprintModifier.ModifierOp = EGameplayModOp::Additive;
    SprintModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-500.f));
    Modifiers.Add(SprintModifier);
}
