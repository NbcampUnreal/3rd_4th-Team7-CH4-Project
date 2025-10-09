// #include "Ability/THClimbAbility.h"
// #include "PlayerCharacter/THPlayerCharacter.h"
// #include "ParkourComponent/THClimbComponent.h"
// #include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
// #include "AbilitySystemComponent.h"
// #include "MotionWarpingComponent.h"
// #include "Components/CapsuleComponent.h"
// #include "Player/THPlayerState.h"
// #include "MotionWarpingComponent.h"
// #include "Game/GameFlowTags.h"
//
// UTHClimbAbility::UTHClimbAbility()
// {
// 	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
// 	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
//
// 	FGameplayTagContainer AbilityTagsContainer;
// 	AbilityTagsContainer.AddTag(TAG_Ability_Climb);
// 	SetAssetTags(AbilityTagsContainer);
//
// 	ActivationOwnedTags.AddTag(TAG_State_Movement_Climbing);
// 	ActivationBlockedTags.AddTag(TAG_Status_Stamina_Empty);
//
// 	BlockAbilitiesWithTag.AddTag(TAG_Ability_Sprint);
// }
//
// bool UTHClimbAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
// {
// 	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
// 	{
// 		return false;
// 	}
// 	
// 	const ATHPlayerCharacter* Character = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get());
// 	if (!Character) return false;
// 	
// 	const UTHClimbComponent* MyClimbComponent = Character->GetClimbComponent();
// 	if (!MyClimbComponent)
// 	{
// 		return false;
// 	}
//
// 	FClimbTraceResult DummyResult;
// 	return MyClimbComponent->CheckClimbableSurface(DummyResult);
// }
//
// void UTHClimbAbility::SetupMotionWarping(const FClimbTraceResult& InClimbTraceResult)
// {
// 	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetAvatarActorFromActorInfo());
// 	if (!PlayerCharacter) return;
// 	
// 	if (UMotionWarpingComponent* MotionWarpingComp = PlayerCharacter->GetMotionWarpingComponent())
// 	{
// 		const float CurrentCapsuleRadius = PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
// 		const FRotator TargetRotation = FRotationMatrix::MakeFromX(-InClimbTraceResult.WallNormal).Rotator();
//
// 		const FVector PlayerProjectedOnWall = FVector::PointPlaneProject(
// 			PlayerCharacter->GetActorLocation(),
// 			InClimbTraceResult.WallLocation,
// 			InClimbTraceResult.WallNormal
// 		);
// 		const FVector TargetPointOnWall = FVector(
// 			PlayerProjectedOnWall.X,
// 			PlayerProjectedOnWall.Y,
// 			InClimbTraceResult.LedgeLocation.Z
// 		);
// 		
// 		const FVector HorizontalOffset = InClimbTraceResult.WallNormal * (CurrentCapsuleRadius - WallInset);
// 		const FVector VerticalOffset = FVector(0.f, 0.f, LedgeGrabVerticalOffset);
// 		const FVector FinalLedgeLocation = TargetPointOnWall + HorizontalOffset + VerticalOffset;
//
// 		FMotionWarpingTarget WarpTargetParams;
// 		WarpTargetParams.Name = FName("LedgeGrab");
// 		WarpTargetParams.Location = FinalLedgeLocation;
// 		WarpTargetParams.Rotation = TargetRotation;
// 		MotionWarpingComp->AddOrUpdateWarpTarget(WarpTargetParams);
// 	}
// }
//
// void UTHClimbAbility::PlayLedgeGrabMontage()
// {
// 	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, LedgeGrabMontage);
// 	if (ActiveMontageTask)
// 	{
// 		ActiveMontageTask->OnCompleted.AddDynamic(this, &UTHClimbAbility::OnLedgeGrabMontageCompleted);
// 		ActiveMontageTask->OnCancelled.AddDynamic(this, &UTHClimbAbility::OnMontageCancelled);
// 		ActiveMontageTask->OnInterrupted.AddDynamic(this, &UTHClimbAbility::OnMontageCancelled);
// 		ActiveMontageTask->ReadyForActivation();
// 	}
// 	else
// 	{
// 		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
// 	}
// }
//
// void UTHClimbAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
// {
// 	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
//
// 	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
// 	{
// 		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
// 		return;
// 	}
//
// 	ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(GetAvatarActorFromActorInfo());
// 	const UTHClimbComponent* ClimbComponent = PlayerCharacter ? PlayerCharacter->GetClimbComponent() : nullptr;
//
// 	if (!ClimbComponent || !ClimbComponent->CheckClimbableSurface(ClimbTraceResult))
// 	{
// 		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
// 		return;
// 	}
// 	
// 	PlayerCharacter->EnterClimbState(ClimbTraceResult.WallNormal);
// 	PlayerCharacter->CacheClimbStaminaEffects(StaminaCostEffect, StaminaRegenEffect);
// 	PlayerCharacter->SwitchClimbStaminaEffect(true);
// 	
// 	SetupMotionWarping(ClimbTraceResult);
// 	
// 	PlayLedgeGrabMontage();
// }
//
// void UTHClimbAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
// {
// 	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
// }
//
// void UTHClimbAbility::OnLedgeGrabMontageCompleted()
// {
// 	ActiveMontageTask = nullptr;
// }
//
// void UTHClimbAbility::OnMontageCancelled()
// {
// 	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
// }
//
// void UTHClimbAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
// {
// 	if (ActiveMontageTask)
// 	{
// 		ActiveMontageTask->EndTask();
// 		ActiveMontageTask = nullptr;
// 	}
//
// 	if (ATHPlayerCharacter* PlayerCharacter = Cast<ATHPlayerCharacter>(ActorInfo->AvatarActor.Get()))
// 	{
// 		if (UMotionWarpingComponent* MotionWarpingComp = PlayerCharacter->GetMotionWarpingComponent())
// 		{
// 			MotionWarpingComp->RemoveWarpTarget(FName("LedgeGrab"));
// 		}
// 		
// 		PlayerCharacter->ClearClimbStaminaEffects();
// 		
// 		if (ATHPlayerState* PS = PlayerCharacter->GetPlayerState<ATHPlayerState>())
// 		{
// 			if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
// 			{
// 				TSubclassOf<UGameplayEffect> DefaultRegenEffect = PS->GetStaminaRegenEffectClass();
// 				if (DefaultRegenEffect)
// 				{
// 					FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
// 					EffectContext.AddSourceObject(this);
// 					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DefaultRegenEffect, 1, EffectContext);
// 					if (SpecHandle.IsValid())
// 					{
// 						ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
// 					}
// 				}
// 			}
// 		}
// 	}
//
// 	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
// }