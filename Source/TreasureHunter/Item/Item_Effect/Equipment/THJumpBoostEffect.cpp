#include "THJumpBoostEffect.h"
#include "GAS/AttributeSet/THAttributeSet.h"
#include "GAS/Tags/GameFlowTags.h"

UTHJumpBoostEffect::UTHJumpBoostEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(30.f);


	FGameplayModifierInfo JumpPowerModifier;
	JumpPowerModifier.Attribute = UTHAttributeSet::GetJumpPowerAttribute();
	JumpPowerModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SBC;
	SBC.DataTag = TAG_Data_JumpDelta;
	JumpPowerModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
	Modifiers.Add(JumpPowerModifier);
}


