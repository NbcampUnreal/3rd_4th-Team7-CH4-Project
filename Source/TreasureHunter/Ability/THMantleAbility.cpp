#include "Ability/THMantleAbility.h"
#include "AttributeSet/THAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "ParkourComponent/THParkourComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/GameFlowTags.h"
#include "MotionWarpingComponent.h"
#include "PlayerCharacter/THPlayerCharacter.h"

UTHMantleAbility::UTHMantleAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	AbilityTags.AddTag(TAG_Ability_Mantle);
	ActivationOwnedTags.AddTag(TAG_Ability_Mantle);
	ActivationBlockedTags.AddTag(TAG_Status_Stamina_Empty);
}

bool UTHMantleAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (const UTHAttributeSet* AS = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass())))
	{
		if (AS->GetStamina() <= KINDA_SMALL_NUMBER)
		{
			return false;
		}
	}
	
	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	const UTHParkourComponent* ParkourComponent = Character ? Character->FindComponentByClass<UTHParkourComponent>() : nullptr;
	if (ParkourComponent)
	{
		FMantleInfo DummyInfo;
		return ParkourComponent->CheckMantle(DummyInfo);
	}

	return false;
}

void UTHMantleAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());
	
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (USpringArmComponent* SpringArm = PlayerCharacter->GetSpringArm())
	{
		SpringArm->bDoCollisionTest = false;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured();
	if (StaminaCostEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaCostEffect, GetAbilityLevel(), EffectContext);
		if (SpecHandle.IsValid())
		{
			(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}

	UTHParkourComponent* ParkourComponent = PlayerCharacter->FindComponentByClass<UTHParkourComponent>();
	FMantleInfo MantleInfo;
    
	if (!ParkourComponent || !ParkourComponent->CheckMantle(MantleInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UMotionWarpingComponent* MotionWarpingComp = PlayerCharacter->MotionWarpingComponent)
	{
		const FName WarpTargetName = FName("MantleTarget"); 
       
		MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
		   WarpTargetName,
		   MantleInfo.TargetLocation,
		   MantleInfo.TargetRotation 
		);
	}
	
	UCharacterMovementComponent* CMC = PlayerCharacter->GetCharacterMovement();
	CMC->SetMovementMode(MOVE_Flying);

	UAnimMontage* MantleMontage = ParkourComponent->GetMantlingMontage();
	if (UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MantleMontage))
	{
		MontageTask->OnCompleted.AddDynamic(this, &UTHMantleAbility::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UTHMantleAbility::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UTHMantleAbility::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UTHMantleAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (USpringArmComponent* SpringArm = PlayerCharacter->GetSpringArm())
			{
				SpringArm->bDoCollisionTest = true;
			}
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTHMantleAbility::OnMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UTHMantleAbility::OnMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}