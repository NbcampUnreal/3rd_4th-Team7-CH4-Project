// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THMatchmakingWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Game/THGameStateBase.h"
#include "Player/THTitlePlayerController.h"
#include "Player/THPlayerState.h"
#include "Game/THGameInstance.h"

void UTHMatchmakingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::OnBackClicked);
	}
	if (FirstPButton)
	{
		FirstPButton->OnClicked.AddDynamic(this, &ThisClass::OnFirstClicked);
	}
	if (SecondPButton)
	{
		SecondPButton->OnClicked.AddDynamic(this, &ThisClass::OnSecondClicked);
	}
	if (MatchStartButton)
	{

		MatchStartButton->OnClicked.AddDynamic(this, &ThisClass::OnStartClicked);
	}

	CachedPC = GetOwningPlayer<ATHTitlePlayerController>();
	CachedGS = GetWorld() ? GetWorld()->GetGameState<ATHGameStateBase>() : nullptr;

	if (CachedGS.IsValid())
	{
		CachedGS->OnSlotsUpdated.AddDynamic(this, &ThisClass::RefreshUI);
	}

	RefreshUI();
}

void UTHMatchmakingWidget::NativeDestruct()
{
	if (CachedGS.IsValid())
	{
		CachedGS->OnSlotsUpdated.RemoveDynamic(this, &ThisClass::RefreshUI);
	}
	Super::NativeDestruct();
}

void UTHMatchmakingWidget::OnBackClicked()
{
	if (!CachedPC.IsValid()) return;
	
	if (auto* THGI = GetWorld() ? GetWorld()->GetGameInstance<UTHGameInstance>() : nullptr)
	{
		if (THGI->bIsHosting)
			THGI->bIsHosting = false;
	}
	CachedPC->BreakMatchSession();
}
void UTHMatchmakingWidget::OnFirstClicked()
{
	if (!CachedPC.IsValid()) return;

	ATHGameStateBase* GS = CachedGS.Get();
	APlayerState* MyPS = CachedPC->GetPlayerState<APlayerState>();
	if (!GS || !MyPS) return;

	const bool bLocked = GS->AreSlotsLockedIn();
	const bool bIAmHost = GS->IsHost(MyPS);

	auto IsSlotReady = [](APlayerState* SlotOwner)->bool
		{
			if (const ATHPlayerState* TH = Cast<ATHPlayerState>(SlotOwner))
			{
				return TH->HasReadyTag();
			}
			return false;
		};

	APlayerState* Owner0 = GS->GetSlotOwner(0);
	const bool bSlot0Ready = IsSlotReady(Owner0);

	const ATHPlayerState* MyTHPS = Cast<ATHPlayerState>(MyPS);
	const bool bIAmReady = (MyTHPS && MyTHPS->HasReadyTag());
	const bool bCanAct =
		!bLocked &&
		bIAmHost &&
		!bIAmReady &&
		!bSlot0Ready &&
		(Owner0 == nullptr || Owner0 == MyPS);

	if (!bCanAct) return;

	CachedPC->Server_TrySelectSlot(0);
	CachedPC->Server_SetReady(true);
}


void UTHMatchmakingWidget::OnSecondClicked()
{
	if (!CachedPC.IsValid()) return;

	ATHGameStateBase* GS = CachedGS.Get();
	APlayerState* MyPS = CachedPC->GetPlayerState<APlayerState>();
	if (!GS || !MyPS) return;

	const bool bLocked = GS->AreSlotsLockedIn();
	const bool bIAmHost = GS->IsHost(MyPS);

	auto IsSlotReady = [](APlayerState* SlotOwner)->bool
		{
			if (const ATHPlayerState* TH = Cast<ATHPlayerState>(SlotOwner))
			{
				return TH->HasReadyTag();
			}
			return false;
		};

	APlayerState* Owner1 = GS->GetSlotOwner(1);
	const bool bSlot1Ready = IsSlotReady(Owner1);

	const ATHPlayerState* MyTHPS = Cast<ATHPlayerState>(MyPS);
	const bool bIAmReady = (MyTHPS && MyTHPS->HasReadyTag());

	const bool bCanAct =
		!bLocked &&
		!bIAmHost &&
		!bIAmReady &&
		!bSlot1Ready &&
		(Owner1 == nullptr || Owner1 == MyPS);

	if (!bCanAct) return;

	CachedPC->Server_TrySelectSlot(1);
	CachedPC->Server_SetReady(true);
}

void UTHMatchmakingWidget::OnStartClicked()
{
	if (CachedPC.IsValid())
	{
		CachedPC->Server_StartMatchIfReady();
	}
}

void UTHMatchmakingWidget::RefreshUI()
{
	ATHGameStateBase* GS = CachedGS.Get();
	if (!GS) return;

	APlayerState* Owner0 = GS->GetSlotOwner(0);
	APlayerState* Owner1 = GS->GetSlotOwner(1);
	APlayerState* MyPS = CachedPC.IsValid() ? CachedPC->GetPlayerState<APlayerState>() : nullptr;

	UpdateNicknameText(FirstPNickname, Owner0, MyPS);
	UpdateNicknameText(SecondPNickname, Owner1, MyPS);

	const bool bLocked = GS->AreSlotsLockedIn();
	const bool bIAmHost = GS->IsHost(MyPS);

	const ATHPlayerState* MyTHPS = MyPS ? Cast<ATHPlayerState>(MyPS) : nullptr;
	const bool bIAmReady = (MyTHPS && MyTHPS->HasReadyTag());

	auto IsSlotReady = [](APlayerState* SlotOwner)->bool
		{
			if (const ATHPlayerState* TH = Cast<ATHPlayerState>(SlotOwner))
			{
				return TH->HasReadyTag();
			}
			return false;
		};

	const bool bSlot0Ready = IsSlotReady(Owner0);
	const bool bSlot1Ready = IsSlotReady(Owner1);

	if (FirstPButton)  FirstPButton->SetIsEnabled(true);
	if (SecondPButton) SecondPButton->SetIsEnabled(true);

	if (bLocked)
	{
		if (FirstPButton)  FirstPButton->SetIsEnabled(false);
		if (SecondPButton) SecondPButton->SetIsEnabled(false);
	}

	if (bSlot0Ready && FirstPButton)  FirstPButton->SetIsEnabled(false);
	if (bSlot1Ready && SecondPButton) SecondPButton->SetIsEnabled(false);

	if (bIAmReady)
	{
		if (bIAmHost && FirstPButton)        FirstPButton->SetIsEnabled(false);
		if (!bIAmHost && SecondPButton)      SecondPButton->SetIsEnabled(false);
	}
		
	if (InviteFriendButton)
	{
		InviteFriendButton->SetVisibility((Owner0 && Owner0 != MyPS) || (Owner1 && Owner1 != MyPS)
			? ESlateVisibility::Collapsed
			: ESlateVisibility::Visible);
	}

	if (MatchStartButton) MatchStartButton->SetIsEnabled(bLocked);
}

void UTHMatchmakingWidget::UpdateNicknameText(UTextBlock* TextWidget, APlayerState* SlotOwner, APlayerState* MyPS)
{
	if (!TextWidget) return;

	if (ATHPlayerState* THPS = Cast<ATHPlayerState>(SlotOwner))
	{
		FString Nick = THPS->Nickname;
		if (THPS == MyPS)
		{
			Nick.Append(TEXT(" (YOU)"));
		}
		TextWidget->SetText(FText::FromString(Nick));
	}
	else
	{
		TextWidget->SetText(FText::GetEmpty());
	}
}
