#include "THShotInkEffect.h"
#include "GAS/AttributeSet/THAttributeSet.h"

UTHShotInkEffect::UTHShotInkEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(2.f);

    FGameplayModifierInfo OverlayWidgetModifier;
    OverlayWidgetModifier.Attribute = UTHAttributeSet::GetOverlayWidgetAttribute();
    OverlayWidgetModifier.ModifierOp = EGameplayModOp::Additive;
    OverlayWidgetModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.f));
    Modifiers.Add(OverlayWidgetModifier);
}