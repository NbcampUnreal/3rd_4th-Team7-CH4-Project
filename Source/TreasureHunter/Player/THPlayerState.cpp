#include "Player/THPlayerState.h"

#include "AttributeSet/THAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"
#include "Game/GameFlowTags.h"

ATHPlayerState::ATHPlayerState()
	: bStartupAbilitiesGiven(false)
{
	bReplicates = true;
	SetNetUpdateFrequency(60.f);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UTHAttributeSet>(TEXT("AttributeSet"));
}

void ATHPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATHPlayerState, Nickname);
}

void ATHPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GiveStartupAbilities();
	}
}

#pragma region GAS

UAbilitySystemComponent* ATHPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UTHAttributeSet* ATHPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void ATHPlayerState::InitializeAbilityActorInfo(APawn* NewPawn)
{
	if (!AbilitySystemComponent || !NewPawn) return;
	AbilitySystemComponent->InitAbilityActorInfo(this, NewPawn);
}

void ATHPlayerState::GiveStartupAbilities()
{
	if (bStartupAbilitiesGiven) return;
	if (!HasAuthority() || !AbilitySystemComponent) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}

	if (StaminaRegenEffect)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StaminaRegenEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	bStartupAbilitiesGiven = true;
}
#pragma endregion

#pragma region Matchmaking

void ATHPlayerState::Server_SetSlotTag(int32 SlotIdx)
{
	check(HasAuthority());
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Player_Character_First);
	AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Player_Character_Second);

	if (SlotIdx == 0)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_Player_Character_First);
	}
	else if (SlotIdx == 1)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_Player_Character_Second);
	}
}

void ATHPlayerState::Server_SetReady(bool bReady)
{
	check(HasAuthority());
	if (!AbilitySystemComponent) return;

	if (bReady)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_Player_Ready);
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Player_Ready);
	}
}

bool ATHPlayerState::HasReadyTag() const
{
	if (const UAbilitySystemComponent* ASC = AbilitySystemComponent)
	{
		return ASC->HasMatchingGameplayTag(TAG_Player_Ready);
	}
	return false;
}
#pragma endregion

#pragma region Nickname
void ATHPlayerState::OnRep_Nickname()
{
	OnNicknameUpdated.Broadcast(Nickname);
}
#pragma endregion