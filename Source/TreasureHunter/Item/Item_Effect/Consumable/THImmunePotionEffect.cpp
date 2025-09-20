#include "THImmunePotionEffect.h"

UTHImmunePotionEffect::UTHImmunePotionEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(5.f);
}

