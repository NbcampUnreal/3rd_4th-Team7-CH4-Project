#include "Item/ItemEffect/THJumpBoostEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"

UTHJumpBoostEffect::UTHJumpBoostEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(30.f);


	FGameplayModifierInfo JumpPowerModifier;
	JumpPowerModifier.Attribute = UTHAttributeSet::GetJumpPowerAttribute();
	JumpPowerModifier.ModifierOp = EGameplayModOp::Additive;
	JumpPowerModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(300.f));
	Modifiers.Add(JumpPowerModifier);
}


