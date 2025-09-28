#include "Ability/THMantleAbility.h"
#include "AttributeSet/THAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "ParkourComponent/THParkourComponent.h"
#include "Game/GameFlowTags.h"
#include "MotionWarpingComponent.h"

UTHMantleAbility::UTHMantleAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	FGameplayTagContainer AbilityTagsContainer;
	AbilityTagsContainer.AddTag(TAG_Ability_Mantle);
	SetAssetTags(AbilityTagsContainer);
	
	ActivationOwnedTags.AddTag(TAG_Status_State_Mantling);
	ActivationBlockedTags.AddTag(TAG_Status_Stamina_Empty);
}

bool UTHMantleAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	const ATHPlayerCharacter* Character = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character) return false;

	const UTHParkourComponent* ParkourComponent = Character->GetParkourComponent();
	if (ParkourComponent)
	{
		FMantleInfo DummyInfo;
		return ParkourComponent->CheckMantle(DummyInfo);
	}

	return false;
}

void UTHMantleAbility::SetupMotionWarping(const FMantleInfo& InMantleInfo)
{
	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetCurrentActorInfo()->AvatarActor.Get());
	if (!PlayerCharacter) return;
	
	if (UMotionWarpingComponent* MotionWarpingComp = PlayerCharacter->GetMotionWarpingComponent())
	{
		FMotionWarpingTarget UpWarpTargetParams;
		UpWarpTargetParams.Name = FName("MantleUp");
		UpWarpTargetParams.Location = InMantleInfo.UpWarpTarget.GetLocation();
		UpWarpTargetParams.Rotation = InMantleInfo.UpWarpTarget.GetRotation().Rotator();
		MotionWarpingComp->AddOrUpdateWarpTarget(UpWarpTargetParams);
		
		FMotionWarpingTarget ForwardWarpTargetParams;
		ForwardWarpTargetParams.Name = FName("MantleForward");
		ForwardWarpTargetParams.Location = InMantleInfo.ForwardWarpTarget.GetLocation();
		ForwardWarpTargetParams.Rotation = InMantleInfo.ForwardWarpTarget.GetRotation().Rotator();
		MotionWarpingComp->AddOrUpdateWarpTarget(ForwardWarpTargetParams);
	}
}

void UTHMantleAbility::PlayMantleMontage()
{
	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetCurrentActorInfo()->AvatarActor.Get());
	if (!PlayerCharacter) return;
	
	UTHParkourComponent* ParkourComponent = PlayerCharacter->GetParkourComponent();
	if (!ParkourComponent) return;

	UAnimMontage* MantleMontage = ParkourComponent->GetMantlingMontage();
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MantleMontage);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UTHMantleAbility::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UTHMantleAbility::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UTHMantleAbility::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
	}
}

void UTHMantleAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured())
	{
		FGameplayTagContainer ClimbAbilityTag(TAG_Ability_Climb);
		ASC->CancelAbilities(&ClimbAbilityTag);
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (StaminaCostEffect)
	{
		(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, MakeOutgoingGameplayEffectSpec(StaminaCostEffect, GetAbilityLevel()));
	}
	
	UTHParkourComponent* ParkourComponent = PlayerCharacter->GetParkourComponent();
	FMantleInfo MantleInfo;
	if (!ParkourComponent || !ParkourComponent->CheckMantle(MantleInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	PlayerCharacter->OnMantleStart();
	SetupMotionWarping(MantleInfo);
	PlayMantleMontage();
}


void UTHMantleAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (const FGameplayAbilityActorInfo* LocalActorInfo = GetCurrentActorInfo())
	{
		if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(LocalActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->OnMantleEnd();
		}
	}
	
	if (SprintCooldownEffect)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(SprintCooldownEffect, GetAbilityLevel());

		(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
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