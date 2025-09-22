#include "GameplayEffect/PlayerStunEffect.h"
#include "NativeGameplayTags.h"
#include "Game/GameFlowTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"


// UPlayerStunEffect::UPlayerStunEffect(const FObjectInitializer& ObjectInitializer)
// 	: Super(ObjectInitializer)
// {
// 	DurationPolicy = EGameplayEffectDurationType::HasDuration;
// 	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat( 10.5f));
// 	
// 	// UTargetTagsGameplayEffectComponent& TargetTags = AddComponent<UTargetTagsGameplayEffectComponent>();
// 	// FInheritedTagContainer TagChanges = TargetTags.GetConfiguredTargetTagChanges();
// 	// TagChanges.Added.AddTag(TAG_State_Debuff_Stun);
// 	// TargetTags.SetAndApplyTargetTagChanges(TagChanges);
//
// 	UTargetTagsGameplayEffectComponent* TargetTagsComponent =
// 		ObjectInitializer.CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(this, FName("TargetTagsComponent"));
//
// 	// 컴포넌트가 성공적으로 생성되었는지 확인
// 	if (TargetTagsComponent)
// 	{
// 		FInheritedTagContainer TagChanges = TargetTagsComponent->GetConfiguredTargetTagChanges();
// 		TagChanges.Added.AddTag(TAG_State_Debuff_Stun);
// 		TargetTagsComponent->SetAndApplyTargetTagChanges(TagChanges);
// 		
// 	}
// }
