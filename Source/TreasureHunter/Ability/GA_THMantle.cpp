// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GA_THMantle.h"
#include "GameFramework/Character.h"
#include "PakourComponent/THCharacterMovementComponent.h"
#include "Game/GameFlowTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "MotionWarpingComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogTH, Log, All);

UGA_THMantle::UGA_THMantle()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(ClimbTags::Ability_Mantle);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(ClimbTags::State_Mantling);
	// Event.Input.Parkour 로 트리거

	FAbilityTriggerData T;
	T.TriggerTag = ClimbTags::Event_Input_Parkour;
	T.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(T);
}

bool UGA_THMantle::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* Info, const FGameplayTagContainer* /*Source*/,
	const FGameplayTagContainer* /*Target*/, FGameplayTagContainer* /*Out*/) const
{
	const ACharacter* Char = Info ? Cast<ACharacter>(Info->AvatarActor.Get()) : nullptr;
	const UTHCharacterMovementComponent* Move = Char ? Cast<UTHCharacterMovementComponent>(Char->GetMovementComponent()) : nullptr;
	if (!Move) return false;

	// 경사/단차 맨틀 조건 확인
	FVector Start, End;
	return Move->CanStartMantle(Start, End);
}

void UGA_THMantle::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* /*Trigger*/)
{
	if (!CommitAbility(Handle, Info, ActivationInfo)) { EndAbility(Handle, Info, ActivationInfo, true, true); return; }

	ACharacter* Char = Cast<ACharacter>(Info->AvatarActor.Get());
	auto* Move = Char ? Cast<UTHCharacterMovementComponent>(Char->GetMovementComponent()) : nullptr;
	if (!Move || !MantleMontage) { EndAbility(Handle, Info, ActivationInfo, true, true); return; }

	FVector MantleStart, MantleEnd;
	if (!Move->CanStartMantle(MantleStart, MantleEnd))
	{
		EndAbility(Handle, Info, ActivationInfo, true, true);
		return;
	}

	// Motion Warping 세팅 
	Move->SetMotionWarpTarget(FName("MantleStartPoint"), MantleStart);
	Move->SetMotionWarpTarget(FName("MantleEndPoint"), MantleEnd);

	// 잠깐 중력 0
	EMovementMode PrevMode = Char->GetCharacterMovement()->MovementMode;
	if (bZeroGravityDuringMantle)
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		Char->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}

	auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MantleMontage, 1.f);
	Task->OnCompleted.AddDynamic(this, &UGA_THMantle::OnMantleMontageEnded);
	Task->OnInterrupted.AddDynamic(this, &UGA_THMantle::OnMantleMontageEnded);
	Task->OnCancelled.AddDynamic(this, &UGA_THMantle::OnMantleMontageEnded);
	Task->ReadyForActivation();
}

void UGA_THMantle::OnMantleMontageEnded()
{
	if (ACharacter* Char = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}