#include "GAS/Ability/THSprintAbility.h"
#include "GAS/AttributeSet/THAttributeSet.h"
#include "Player/PlayerCharacter/THPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GAS/Tags/GameFlowTags.h"

UTHSprintAbility::UTHSprintAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer Tags = GetAssetTags();
	Tags.AddTag(TAG_Ability_Sprint);
	SetAssetTags(Tags);

	ActivationOwnedTags.AddTag(TAG_State_Movement_Sprinting);
	//ActivationBlockedTags.AddTag(TAG_Status_State_Mantling);
	ActivationBlockedTags.AddTag(TAG_Status_Stamina_Empty);
}

bool UTHSprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	//if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	//{
	//	return false;
	//}
	//
	//const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	//if (!ensure(ASC))
	//{
	//	return false;
	//}

	//if (const UTHAttributeSet* AttributeSet = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass())))
	//{
	//	return AttributeSet->GetStamina() > KINDA_SMALL_NUMBER;
	//}
	//ensure(false);
	//
	//return false;
}

void UTHSprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // [FIX]
		
		return;
	}

	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		// [OPTION] 지속 코스트가 필요하면 주기형 GE 사용 권장. 여기선 즉시 코스트 예시.
		if (StaminaCostEffect)
		{
			FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
			Ctx.AddSourceObject(this);
			if (FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaCostEffect, GetAbilityLevel(), Ctx); Spec.IsValid())
			{
				StaminaCostEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}

		// [FIX] 스태미나 변경 감지 → 0에 가까우면 스프린트 취소
		StaminaChangeDelegateHandle =
			ASC->GetGameplayAttributeValueChangeDelegate(UTHAttributeSet::GetStaminaAttribute())
			.AddUObject(this, &UTHSprintAbility::OnStaminaChanged);
	}
}

void UTHSprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (StaminaChangeDelegateHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UTHAttributeSet::GetStaminaAttribute())
				.Remove(StaminaChangeDelegateHandle);
			StaminaChangeDelegateHandle.Reset();
		}

		// [OPTION] 종료 시 회복 버프 부여
		if (StaminaRegenEffect)
		{
			FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
			Ctx.AddSourceObject(this);
			if (FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaRegenEffect, GetAbilityLevel(), Ctx); Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTHSprintAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void UTHSprintAbility::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= KINDA_SMALL_NUMBER)
	{
		// [FIX] 스태미나 0이면 능력 강제 취소
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	}
}