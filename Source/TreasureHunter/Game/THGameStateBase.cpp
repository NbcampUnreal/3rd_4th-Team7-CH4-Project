// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/THGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Game/GameFlowTags.h"

void ATHGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATHGameStateBase, PhaseTag);
}

void ATHGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && PhaseTag.IsValid() == false)
	{
		SetPhase(TAG_Game_Phase_Wait);
	}
}

void ATHGameStateBase::SetPhase(const FGameplayTag& NewPhase)
{
	if (HasAuthority() && PhaseTag != NewPhase)
	{
		PhaseTag = NewPhase;
		OnRep_PhaseTag();
		ForceNetUpdate();
	}
}

void ATHGameStateBase::OnRep_PhaseTag()
{
	OnPhaseChanged.Broadcast(PhaseTag);
}
