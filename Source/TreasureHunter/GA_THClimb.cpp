// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_THClimb.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "PakourComponent/THCharacterMovementComponent.h"
#include "Game/GameFlowTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

DEFINE_LOG_CATEGORY_STATIC(LogTH, Log, All);

UGA_THClimb::UGA_THClimb()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(ClimbTags::Ability_Climb);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(ClimbTags::State_Climbing);

	FAbilityTriggerData T;
	T.TriggerTag = ClimbTags::Event_Input_Climb;
	T.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(T);
}

bool UGA_THClimb::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* Info, const FGameplayTagContainer* /*SourceTags*/,
	const FGameplayTagContainer* /*TargetTags*/, FGameplayTagContainer* /*Out*/) const
{
	if (!Info || !Info->AvatarActor.IsValid()) return false;

	const ACharacter* Char = Cast<ACharacter>(Info->AvatarActor.Get());
	if (!Char) return false;

	UTHCharacterMovementComponent* Move = Cast<UTHCharacterMovementComponent>(Char->GetMovementComponent());
	if (!Move) return false;

	// 등반 시작 가능 or 등반 중(토글) or ledge 내려가기
	return Move->IsClimbing() || Move->CanStartClimbing() || Move->CanClimbDownLedge();
}

void UGA_THClimb::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, Info, ActivationInfo)) { EndAbility(Handle, Info, ActivationInfo, true, true); return; }

	ACharacter* Char = Cast<ACharacter>(Info->AvatarActor.Get());
	auto* Move = Char ? Cast<UTHCharacterMovementComponent>(Char->GetMovementComponent()) : nullptr;
	if (!Move) { EndAbility(Handle, Info, ActivationInfo, true, true); return; }

	UAnimMontage* MontageToPlay = nullptr;

	if (Move->IsClimbing())
	{
		// 토글: 내려오기 (낙하 or 걷기로)
		Move->ToggleClimbing(false);
		EndAbility(Handle, Info, ActivationInfo, false, false);
		return;
	}
	else if (Move->CanStartClimbing())
	{
		MontageToPlay = Move->GetIdleToClimbMontage();
	}
	else if (Move->CanClimbDownLedge())
	{
		MontageToPlay = Move->GetClimbDownLedgeMontage();
	}

	if (!MontageToPlay)
	{
		EndAbility(Handle, Info, ActivationInfo, true, true);
		return;
	}

	// 몽타주는 AbilityTask로 재생중 
	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay, 1.f);
	Task->OnCompleted.AddDynamic(this, &UGA_THClimb::OnMontageCompleted);
	Task->OnInterrupted.AddDynamic(this, &UGA_THClimb::OnMontageCompleted);
	Task->OnCancelled.AddDynamic(this, &UGA_THClimb::OnMontageCompleted);
	Task->ReadyForActivation();
}

void UGA_THClimb::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}