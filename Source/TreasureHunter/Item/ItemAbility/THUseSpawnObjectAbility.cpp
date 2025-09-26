#include "Item/ItemAbility/THUseSpawnObjectAbility.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Item/Spawn/THSpawnObject.h"



bool UTHUseSpawnObjectAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor || !AvatarActor->GetWorld() || !ObjectClass)
	{
		return false;
	}

	if (MaxCreateObjectHeight == 0)
	{
		return true;
	}

	FVector StartLocation = AvatarActor->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0, 0, MaxCreateObjectHeight);
	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = false;
	TraceParams.AddIgnoredActor(AvatarActor);
		
	return AvatarActor->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams);
}


void UTHUseSpawnObjectAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
		
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor || !AvatarActor->GetWorld() || !ObjectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	FVector StartLocation = AvatarActor->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0, 0, 500.f);
	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = false;
	TraceParams.AddIgnoredActor(AvatarActor);

	FVector SpawnLocation = AvatarActor->GetActorLocation();
	if (AvatarActor->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams))
	{
		SpawnLocation = HitResult.ImpactPoint;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ObjectClass, SpawnLocation, AvatarActor->GetActorRotation(), SpawnParams);
	
	if (SpawnedActor)
	{
		ATHSpawnObject* SpawnedTrap = Cast<ATHSpawnObject>(SpawnedActor);
		if (SpawnedTrap)
		{
			SpawnedTrap->SetPlacerActor(AvatarActor);
			
		}
	}
		
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

