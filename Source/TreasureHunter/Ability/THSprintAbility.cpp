#include "Ability/THSprintAbility.h"
#include "AttributeSet/THAttributeSet.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Game/GameFlowTags.h"

UTHSprintAbility::UTHSprintAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(TAG_Ability_Sprint);
	ActivationOwnedTags.AddTag(TAG_State_Movement_Sprinting);
	ActivationBlockedTags.AddTag(TAG_Status_State_Mantling);
	ActivationBlockedTags.AddTag(TAG_Status_Stamina_Empty);
}

bool UTHSprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ensure(ASC))
	{
		return false;
	}

	if (const UTHAttributeSet* AttributeSet = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass())))
	{
		return AttributeSet->GetStamina() > KINDA_SMALL_NUMBER;
	}
	ensure(false);
	
	return false;
}

void UTHSprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		
		return;
	}

	
	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());
	
	checkf(PlayerCharacter, TEXT("ATHPlayerCharacter is null in UTHSprintAbility."));
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured();

	PlayerCharacter->bIsSprinting = true;
	
	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = PlayerCharacter->GetSprintSpeed();
	
	if (ensure(StaminaRegenEffect))
	{
		const UGameplayEffect* RegenEffectCDO = StaminaRegenEffect.GetDefaultObject();

		checkf(RegenEffectCDO, TEXT("StaminaRegenEffect CDO is null."));

		FGameplayTagContainer RegenTags = RegenEffectCDO->GetGrantedTags();

		ASC->RemoveActiveEffectsWithGrantedTags(RegenTags);
	}

	if (ASC->GetOwnerRole() == ROLE_Authority && ensure(StaminaCostEffect))
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaCostEffect, 1, EffectContext);

		if (SpecHandle.IsValid())
		{
			StaminaCostEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
	if (const UTHAttributeSet* AttributeSet = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass())); ensure(AttributeSet))
	{
		StaminaChangeDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute()).AddUObject(this, &UTHSprintAbility::OnStaminaChanged);
	}
}

void UTHSprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());

		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		
		if (PlayerCharacter && ASC)
		{
			PlayerCharacter->bIsSprinting = false;

			PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = PlayerCharacter->GetWalkSpeed();
			
			if (StaminaCostEffectHandle.IsValid())
			{
				if (ASC->GetOwnerRole() == ROLE_Authority)
				{
					ASC->RemoveActiveGameplayEffect(StaminaCostEffectHandle);
				}
			}
			
			if (ensure(StaminaRegenEffect))
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();

				EffectContext.AddSourceObject(this);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaRegenEffect, 1, EffectContext);

				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}

			if (StaminaChangeDelegateHandle.IsValid())
			{
				if (const UTHAttributeSet* AttributeSet = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass())))
				{
					ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute()).Remove(StaminaChangeDelegateHandle);
				}
				StaminaChangeDelegateHandle.Reset();
			}
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTHSprintAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTHSprintAbility::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= KINDA_SMALL_NUMBER)
	{
		if (auto* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddLooseGameplayTag(TAG_Status_Stamina_Empty);
		}

		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
	}
}