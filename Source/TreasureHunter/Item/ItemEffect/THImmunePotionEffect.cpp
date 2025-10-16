#include "Item/ItemEffect/THImmunePotionEffect.h"
#include "Game/GameFlowTags.h"
#include "GameplayEffect.h"



UTHImmunePotionEffect::UTHImmunePotionEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(5.f);
	    
	FGameplayEffectCue Cue;
	Cue.GameplayCueTags.AddTag(TAG_Cue_ImmunePotion);
	GameplayCues.Add(Cue);
}

