#include "GAS/Effect/CoolDownEffect_Push.h"
#include "GAS/Tags/GameFlowTags.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"


UCoolDownEffect_Push::UCoolDownEffect_Push()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	DurationMagnitude = FScalableFloat(10.5f);
	
	InheritableOwnedTagsContainer.Added.AddTag(TAG_Cooldown_Ability_Push);
	
}
