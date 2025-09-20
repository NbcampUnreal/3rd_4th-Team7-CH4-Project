#include "THStaminaRecoveryEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "Game/GameFlowTags.h"

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


