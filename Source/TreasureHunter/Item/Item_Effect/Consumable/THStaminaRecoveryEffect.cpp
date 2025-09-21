#include "THStaminaRecoveryEffect.h"
#include "GAS/AttributeSet/THAttributeSet.h"
#include "GAS/Tags/GameFlowTags.h"

UTHStaminaRecoveryEffect::UTHStaminaRecoveryEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UTHAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SBC;
	SBC.DataTag = TAG_Data_StaminaDelta;
	StaminaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);

	Modifiers.Add(StaminaModifier);
}


