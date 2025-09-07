#include "Player/THPlayerState.h"
#include "AttributeSet/THAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

ATHPlayerState::ATHPlayerState()
{
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UTHAttributeSet>(TEXT("AttributeSet"));
	
}

UAbilitySystemComponent* ATHPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UTHAttributeSet* ATHPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void ATHPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		for (const auto& AbilityClass : StartupAbilities)
		{
			if (AbilityClass)
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1, 0, this);
				AbilitySystemComponent->GiveAbility(Spec);
			}
		}
		
		if (StaminaRegenEffect)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StaminaRegenEffect, 1, EffectContext);

			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}