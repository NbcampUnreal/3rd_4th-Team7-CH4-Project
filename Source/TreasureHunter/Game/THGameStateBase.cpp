// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/THGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Game/GameFlowTags.h"
#include "Player/THPlayerState.h"

void ATHGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority()) return;

	if (PhaseTag.IsValid() == false)
	{
		SetPhase(TAG_Game_Phase_Wait);
	}

	if (SlotOwners.Num() != 2)
	{
		SlotOwners.SetNum(2);
		SlotOwners[0] = nullptr;
		SlotOwners[1] = nullptr;
	}
	bSlotsLockedIn = false;
}

void ATHGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATHGameStateBase, PhaseTag);
	DOREPLIFETIME(ATHGameStateBase, WinnerTag);
	DOREPLIFETIME(ATHGameStateBase, SlotOwners);
	DOREPLIFETIME(ATHGameStateBase, bSlotsLockedIn);

	DOREPLIFETIME(ATHGameStateBase, RematchTag);
	DOREPLIFETIME(ATHGameStateBase, RematchRequester);
	DOREPLIFETIME(ATHGameStateBase, RematchResponder);
	DOREPLIFETIME(ATHGameStateBase, RematchAcceptMask);
	DOREPLIFETIME(ATHGameStateBase, RematchExpireAt);
}

#pragma region Phase
void ATHGameStateBase::SetPhase(const FGameplayTag& NewPhase)
{
	if (!HasAuthority()) return;

	if (PhaseTag != NewPhase)
	{
		PhaseTag = NewPhase;

		if (NewPhase.MatchesTagExact(TAG_Game_Phase_Match))
		{
			ResetSlots();
		}

		OnRep_PhaseTag();
		ForceNetUpdate();
	}
}

void ATHGameStateBase::SetWinnerTag(const FGameplayTag& NewWinner)
{
	if (!HasAuthority()) return;

	WinnerTag = NewWinner;
	ForceNetUpdate();
}

void ATHGameStateBase::OnRep_PhaseTag()
{
	OnPhaseChanged.Broadcast(PhaseTag);
}

#pragma endregion

#pragma region Matchmaking

APlayerState* ATHGameStateBase::GetSlotOwner(int32 SlotIdx) const
{
	return SlotOwners.IsValidIndex(SlotIdx) ? SlotOwners[SlotIdx] : nullptr;
}

bool ATHGameStateBase::AreSlotsFilled() const
{
	return SlotOwners.Num() == 2 && SlotOwners[0] != nullptr && SlotOwners[1] != nullptr && SlotOwners[0] != SlotOwners[1];
}

bool ATHGameStateBase::AreBothReady() const
{
	if (SlotOwners.Num() < 2) return false;
	auto* P0 = Cast<ATHPlayerState>(SlotOwners[0]);
	auto* P1 = Cast<ATHPlayerState>(SlotOwners[1]);
	return (P0 && P1 && P0->HasReadyTag() && P1->HasReadyTag());
}

bool ATHGameStateBase::TryAssignSlot(int32 SlotIdx, APlayerState* Requestor)
{
	if (!HasAuthority() || !Requestor || !SlotOwners.IsValidIndex(SlotIdx) || bSlotsLockedIn)
		return false;

	const bool bRequestorIsHost = (Requestor == HostPS);
	if ((bRequestorIsHost && SlotIdx != 0) || (!bRequestorIsHost && SlotIdx != 1)) return false;
	if (SlotOwners[SlotIdx] && SlotOwners[SlotIdx] != Requestor) return false;

	if (SlotOwners[0] == Requestor) SlotOwners[0] = nullptr;
	if (SlotOwners[1] == Requestor) SlotOwners[1] = nullptr;

	SlotOwners[SlotIdx] = Requestor;
	if (ATHPlayerState* THPS = Cast<ATHPlayerState>(Requestor))
	{
		THPS->Server_SetSlotTag(SlotIdx);
		THPS->Server_SetReady(false);
	}

	bSlotsLockedIn = AreSlotsFilled() && AreBothReady();

	OnRep_SlotOwners();
	OnRep_SlotsLockedIn();
	ForceNetUpdate();
	return true;
}

void ATHGameStateBase::ResetSlots()
{
	if (!HasAuthority()) return;

	SlotOwners.SetNum(2);
	SlotOwners[0] = nullptr;
	SlotOwners[1] = nullptr;
	bSlotsLockedIn = false;

	OnRep_SlotOwners();
	OnRep_SlotsLockedIn();
	ForceNetUpdate();
}


void ATHGameStateBase::OnRep_SlotOwners()
{
	OnSlotsUpdated.Broadcast();
}

void ATHGameStateBase::OnRep_SlotsLockedIn()
{
	OnSlotsUpdated.Broadcast();
}
#pragma endregion

#pragma region Rematch
void ATHGameStateBase::OnRep_RematchTag()
{
	OnRematchChanged.Broadcast(RematchTag);
}

void ATHGameStateBase::ResetRematchState() // Initialize when Phase Finish
{
	RematchTag = FGameplayTag();
	RematchRequester = nullptr;
	RematchResponder = nullptr;
	RematchAcceptMask = 0;
	RematchExpireAt = 0.f;
	OnRep_RematchTag();
	ForceNetUpdate();
}
void ATHGameStateBase::SetRematchTag(const FGameplayTag& RematchRequest)
{
	if (!HasAuthority()) return;

	if (RematchTag != RematchRequest)
	{
		RematchTag = RematchRequest;

		OnRep_RematchTag();
		ForceNetUpdate();
	}
}
void ATHGameStateBase::SetRematchRequester(APlayerState* Requester)
{
	if (!HasAuthority()) return;

	RematchRequester = Requester;
}
void ATHGameStateBase::SetRematchResponder(APlayerState* Responder)
{
	if (!HasAuthority()) return;

	RematchResponder = Responder;
}

void ATHGameStateBase::OnRep_HostPS()
{
	OnSlotsUpdated.Broadcast();
}
#pragma endregion