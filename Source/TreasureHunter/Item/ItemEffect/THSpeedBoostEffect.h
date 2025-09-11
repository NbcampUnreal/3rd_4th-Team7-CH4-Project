#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "THSpeedBoostEffect.generated.h"


UCLASS()
class TREASUREHUNTER_API UTHSpeedBoostEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UTHSpeedBoostEffect()
	{
		DurationPolicy = EGameplayEffectDurationType::HasDuration;
		DurationMagnitude=FScalableFloat(10.0f); // 10초 지속

		FGameplayModifierInfo WalkModifier;
		WalkModifier.Attribute = UTHAttributeSet::GetWalkSpeedAttribute();
		WalkModifier.ModifierOp = EGameplayModOp::Additive;
		WalkModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(200.0f));
		Modifiers.Add(WalkModifier);

		FGameplayModifierInfo SprintModifier;
		SprintModifier.Attribute = UTHAttributeSet::GetSprintSpeedAttribute();
		SprintModifier.ModifierOp = EGameplayModOp::Additive;
		SprintModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(500.0f));
		Modifiers.Add(SprintModifier);
	}
	
};
