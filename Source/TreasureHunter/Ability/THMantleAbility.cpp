#include "Ability/THMantleAbility.h"
#include "PlayerCharacter/THPlayerCharacter.h"
#include "ParkourComponent/THParkourComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"


UTHMantleAbility::UTHMantleAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Mantle")));
}

bool UTHMantleAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (!ensure(Character) || !ensure(ASC))
	{
		return false;
	}

	if (Character->GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}

	const UTHParkourComponent* ParkourComponent = Character->FindComponentByClass<UTHParkourComponent>();

	FMantleInfo TempMantleInfo;

	if (!ParkourComponent || !ParkourComponent->CheckMantle(TempMantleInfo))
	{
		return false;
	}
}

void UTHMantleAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	UTHParkourComponent* ParkourComponent = Character ? Character->FindComponentByClass<UTHParkourComponent>() : nullptr;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured();

	if (!ParkourComponent || !ParkourComponent->CheckMantle(MantleInfo))
	{
		constexpr bool bReplicateEndAbility = true;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}

	if (IsValid(StaminaCostEffect) == true)
	{
		ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, StaminaCostEffect.GetDefaultObject(), GetAbilityLevel());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("'%s' is missing StaminaCostEffect."), *GetName());
	}

	UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement();

	CharacterMovementComponent->SetMovementMode(MOVE_Flying);

	CharacterMovementComponent->StopMovementImmediately();

	UAnimMontage* MantleMontage = ParkourComponent->GetMantlingMontage();

	if (IsValid(MantleMontage) == true)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MantleMontage);
		MontageTask->OnCompleted.AddDynamic(this, &UTHMantleAbility::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UTHMantleAbility::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UTHMantleAbility::OnMontageEnded);
		MontageTask->ReadyForActivation();

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(
			Character->GetCapsuleComponent(), MantleInfo.TargetLocation, MantleInfo.TargetRotation,
			true, true, 0.2f, false, EMoveComponentAction::Type::Move, LatentInfo);
	}
	else
	{
		constexpr bool bReplicateEndAbility = true;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UTHMantleAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		
		if (IsValid(Character) == true)
		{
			UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
			if (IsValid(MovementComponent && MovementComponent->IsFlying()) == true)
			{
				MovementComponent->SetMovementMode(MOVE_Walking);
			}
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UTHMantleAbility::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	constexpr bool bReplicateEndAbility = true;
	const bool bWasCancelled = bInterrupted;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), bReplicateEndAbility, bWasCancelled);
}
