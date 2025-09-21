#include "GAS/AttributeSet/THAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GAS/Tags/GameFlowTags.h"

UTHAttributeSet::UTHAttributeSet()
{
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitWalkSpeed(200.f);
	InitSprintSpeed(600.f);
	InitJumpPower(420.f);
	InitOverlayWidget(0.f);
}

void UTHAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTHAttributeSet, Stamina, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTHAttributeSet, MaxStamina, COND_OwnerOnly, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UTHAttributeSet, WalkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTHAttributeSet, SprintSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTHAttributeSet, JumpPower, COND_None, REPNOTIFY_Always);
}

void UTHAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		if (auto* ASC = GetOwningAbilitySystemComponent())
		{
			if (GetStamina() <= KINDA_SMALL_NUMBER)
			{
				ASC->AddLooseGameplayTag(TAG_Status_Stamina_Empty);
			}
			else
			{
				ASC->RemoveLooseGameplayTag(TAG_Status_Stamina_Empty);
			}
		}
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
}

void UTHAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTHAttributeSet, Stamina, OldStamina)
}

void UTHAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTHAttributeSet, MaxStamina, OldMaxStamina);
}

void UTHAttributeSet::OnRep_WalkSpeed(const FGameplayAttributeData& OldWalkSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTHAttributeSet, WalkSpeed, OldWalkSpeed);
}

void UTHAttributeSet::OnRep_SprintSpeed(const FGameplayAttributeData& OldSprintSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTHAttributeSet, SprintSpeed, OldSprintSpeed);
}

void UTHAttributeSet::OnRep_JumpPower(const FGameplayAttributeData& OldJumpPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTHAttributeSet, JumpPower, OldJumpPower);
}

void UTHAttributeSet::OnRep_OverlayWidget(const FGameplayAttributeData& OldOverlayWidget)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTHAttributeSet, OverlayWidget, OldOverlayWidget);
}



