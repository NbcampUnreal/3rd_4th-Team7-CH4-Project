// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THMatchmakingWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Game/THGameStateBase.h"
#include "Player/THTitlePlayerController.h"
#include "Player/THPlayerState.h"

void UTHMatchmakingWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

void UTHMatchmakingWidget::OnFirstClicked()
{
	if (CachedPC.IsValid())
	{
		CachedPC->Server_TrySelectSlot(0);
		CachedPC->Server_SetReady(true);
	}
}

void UTHMatchmakingWidget::OnSecondClicked()
{
	if (CachedPC.IsValid())
	{
		CachedPC->Server_TrySelectSlot(1);
		CachedPC->Server_SetReady(true);
	}
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

	if (FirstPButton) FirstPButton->SetIsEnabled(!bLocked && (Owner0 == nullptr));
	if (SecondPButton) SecondPButton->SetIsEnabled(!bLocked && (Owner1 == nullptr));

	if (UnlockImage) UnlockImage->SetVisibility(bLocked ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	if (LockImage)   LockImage->SetVisibility(bLocked ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
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
