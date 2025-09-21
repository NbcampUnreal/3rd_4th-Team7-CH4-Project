#include "THSpeedSlowEffect.h"
#include "GAS/AttributeSet/THAttributeSet.h"
#include "GAS/Tags/GameFlowTags.h"

UTHSpeedSlowEffect::UTHSpeedSlowEffect()
{
    UE_LOG(LogTemp, Log, TEXT("[THSpeedSlowEffect::CTOR] ..."));
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(5.f);

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
