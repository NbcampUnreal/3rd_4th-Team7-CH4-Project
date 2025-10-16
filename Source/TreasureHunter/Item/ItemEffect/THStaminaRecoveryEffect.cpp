#include "Item/ItemEffect/THStaminaRecoveryEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"

UTHStaminaRecoveryEffect::UTHStaminaRecoveryEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UTHAttributeSet::GetStaminaAttribute();

	StaminaModifier.ModifierOp = EGameplayModOp::Additive;

	StaminaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(20.f));

	Modifiers.Add(StaminaModifier);
}


