#include "THShotRocketEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "Game/GameFlowTags.h"

UTHShotRocketEffect::UTHShotRocketEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(2.f);

    // Walk
    {
        FGameplayModifierInfo M;
        M.Attribute = UTHAttributeSet::GetWalkSpeedAttribute();
        M.ModifierOp = EGameplayModOp::Additive;

        FSetByCallerFloat SBC;
        SBC.DataTag = TAG_Data_WalkDelta;
        M.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
        Modifiers.Add(M);
    }
    // Sprint
    {
        FGameplayModifierInfo M;
        M.Attribute = UTHAttributeSet::GetSprintSpeedAttribute();
        M.ModifierOp = EGameplayModOp::Additive;

        FSetByCallerFloat SBC;
        SBC.DataTag = TAG_Data_SprintDelta;
        M.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
        Modifiers.Add(M);
    }

}
