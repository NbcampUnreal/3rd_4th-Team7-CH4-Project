#include "Player/THPlayerState.h"

#include "AttributeSet/THAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"
#include "Game/GameFlowTags.h"
#include "Game/THGameStateBase.h"

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

	if (ATHGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->bSlotsLockedIn = GS->AreSlotsFilled() && GS->AreBothReady();
		GS->Multicast_OnSlotsUpdated();
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

	if (ATHGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr)
	{
		GS->OnSlotsUpdated.Broadcast();
	}
}
void ATHPlayerState::OrganizeAbilitySystemComponent()
{
	if (!IsValid(AbilitySystemComponent)) return;

	AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().RemoveAll(nullptr);
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(nullptr);

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);

	for (const FGameplayTag& Tag : OwnedTags)
	{
		// 기존 델리게이트만 제거
		AbilitySystemComponent
			->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
			.RemoveAll(this);
	}
	
	/*AbilitySystemComponent->CancelAllAbilities();
	AbilitySystemComponent->ClearAllAbilities();
	AbilitySystemComponent->RemoveAllGameplayCues();

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
	for (const FGameplayTag& Tag : OwnedTags)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(Tag);
	}

	AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.RemoveAll(this);

	AbilitySystemComponent->InitAbilityActorInfo(nullptr, nullptr);*/
	
	//AbilitySystemComponent->DestroyComponent();
}
#pragma endregion

#pragma region APlayerStateData
void ATHPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (ATHPlayerState* NewPS = Cast<ATHPlayerState>(PlayerState))
	{
		NewPS->PlayerUniqueId = PlayerUniqueId;
		NewPS->Nickname = Nickname;

		if (AbilitySystemComponent)
		{
			FGameplayTagContainer TempTags;
			AbilitySystemComponent->GetOwnedGameplayTags(TempTags);

			if (NewPS->AbilitySystemComponent)
			{
				for (const FGameplayTag& Tag : TempTags)
				{
					NewPS->AbilitySystemComponent->AddLooseGameplayTag(Tag);
					UE_LOG(LogTemp, Error, TEXT("%s"), *Tag.ToString());
				}
			}
		}
	}
}
#pragma endregion