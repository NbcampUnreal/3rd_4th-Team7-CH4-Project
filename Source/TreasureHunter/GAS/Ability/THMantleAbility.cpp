#include "GAS/Ability/THMantleAbility.h"
#include "GAS/AttributeSet/THAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Player/PlayerCharacter/THPlayerCharacter.h"
#include "Player/Components/THParkourComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GAS/Tags/GameFlowTags.h"
#include "MotionWarpingComponent.h"

UTHMantleAbility::UTHMantleAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	AbilityTags.AddTag(TAG_Ability_Mantle);
	ActivationOwnedTags.AddTag(TAG_Status_State_Mantling);
	ActivationBlockedTags.AddTag(TAG_Status_Stamina_Empty);
}

bool UTHMantleAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	//const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	//if (const UTHAttributeSet* AS = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass())))
	//{
	//	if (AS->GetStamina() <= KINDA_SMALL_NUMBER)
	//	{
	//		return false;
	//	}
	//}
	
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

	// [FIX] ���� �̵� ���/��������/ĸ�� ���� ���
	if (UCharacterMovementComponent* CMC = PlayerCharacter->GetCharacterMovement())
	{
		SavedMovementMode = CMC->MovementMode;
		SavedCustomMovementMode = CMC->CustomMovementMode;
		CMC->SetMovementMode(MOVE_Flying); // [FIX] ��Ʋ �� �浹/���� ���� ����
	}
	if (UCapsuleComponent* Capsule = PlayerCharacter->GetCapsuleComponent())
	{
		SavedCapsuleProfile = Capsule->GetCollisionProfileName();
		Capsule->SetCollisionProfileName(FName("Mantling"));

	}
	
	if (USpringArmComponent* SpringArm = PlayerCharacter->GetSpringArm())
	{
		bSpringArmCollisionSaved = SpringArm->bDoCollisionTest;
		SpringArm->bDoCollisionTest = false;
	}
	
	// [FIX] ASC üũ 
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured())
	{
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
		FMotionWarpingTarget UpWarpTargetParams;
		UpWarpTargetParams.Name = FName("MantleUp");
		UpWarpTargetParams.Location = MantleInfo.UpWarpTarget.GetLocation();
		UpWarpTargetParams.Rotation = MantleInfo.UpWarpTarget.GetRotation().Rotator();
		
		MotionWarpingComp->AddOrUpdateWarpTarget(UpWarpTargetParams);
		
		FMotionWarpingTarget ForwardWarpTargetParams;
		ForwardWarpTargetParams.Name = FName("MantleForward");
		ForwardWarpTargetParams.Location = MantleInfo.ForwardWarpTarget.GetLocation();
		ForwardWarpTargetParams.Rotation = MantleInfo.ForwardWarpTarget.GetRotation().Rotator();

		MotionWarpingComp->AddOrUpdateWarpTarget(ForwardWarpTargetParams);
	}
	
	UAnimMontage* MantleMontage = ParkourComponent->GetMantlingMontage();
	if (UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MantleMontage))
	{
		MontageTask->OnCompleted.AddDynamic(this, &UTHMantleAbility::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UTHMantleAbility::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UTHMantleAbility::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
		return;
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UTHMantleAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (const FGameplayAbilityActorInfo* LocalInfo = GetCurrentActorInfo())
	{
		if (ATHPlayerCharacter* PC = Cast<ATHPlayerCharacter>(LocalInfo->AvatarActor.Get()))
		{
			// [FIX] �������� �浹 ����
			if (USpringArmComponent* SpringArm = PC->GetSpringArm())
			{
				SpringArm->bDoCollisionTest = bSpringArmCollisionSaved;
			}
			// [FIX] ĸ�� �������� ����
			if (UCapsuleComponent* Capsule = PC->GetCapsuleComponent())
			{
				if (SavedCapsuleProfile != NAME_None)
				{
					Capsule->SetCollisionProfileName(SavedCapsuleProfile);
				}
				else
				{
					Capsule->SetCollisionProfileName(FName("Pawn"));
				}
			}
			// [FIX] ���� Ÿ�� ����
			if (UMotionWarpingComponent* MW = PC->MotionWarpingComponent)
			{
				MW->RemoveWarpTarget(FName("MantleUp"));
				MW->RemoveWarpTarget(FName("MantleForward"));
			}
			// [FIX] �̵� ��� ����
			if (UCharacterMovementComponent* CMC = PC->GetCharacterMovement())
			{
				CMC->SetMovementMode(SavedMovementMode, SavedCustomMovementMode);
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