// Fill out your copyright notice in the Description page of Project Settings.


#include "THPlayerHUDWidget.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "TimerManager.h"
#include "Game/GameFlowTags.h"
#include "Item/Item_Data/THItemDataManager.h"
#include "Item/Item_Data/THItemData.h"

DEFINE_LOG_CATEGORY_STATIC(LogTHHUD, Log, All);

#pragma region General
UTHPlayerHUDWidget::UTHPlayerHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	// --- Stamina ---
	, StaminaBaseWidth(500.f)
	, StaminaInitialMax(100.f)
	, StaminaDisplayedPercent(1.f)
	, StaminaAnimTickInterval(0.02f)
	, StaminaAnimSpeedDown(2.0f)
	, StaminaAnimSpeedUp(1.0f)
	, bIsAnimatingStamina(false)
	, AnimToPercent(1.f)
	, AnimSpeedCurrent(1.f)
	, bIsSprinting(false)
	// --- Status Bars (Speed/Jump) ---
	, SpeedBaseValue(200.f)
	, SpeedStep(200.f)
	, JumpBaseValue(420.f)
	, JumpStep(100.f)
	// --- Blink ---
	, BlinkThresholdSec(3.f)
	, BlinkIntervalSec(0.5f)
	// --- Inventory ---
	, DurationTotalSec(0.f)
	, DurationElapsedSec(0.f)
	, ActiveDebuffOverlay(nullptr)
	// --- Climb ---
	, bHasBunnyBeenWinning(false)
	, OppoBaseY(0.f)
	, OppoTravelDeltaY(-445.f)
	, TargetSelfP(0.f)
	, TargetOppoP(0.f)
	, DisplayedSelfP(0.f)
	, DisplayedOppoP(0.f)
	// --- ASC / Attr ---
	, Attr(nullptr)
	, AbilitySystem(nullptr)
{
}

void UTHPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CacheStatusBars();

	if (TopRightBuffIcon)
	{
		TopRightBuffIcon->SetVisibility(ESlateVisibility::Hidden);
	}
	if (TopRightBuffIconBG)
	{
		TopRightBuffIconBG->SetVisibility(ESlateVisibility::Hidden);
	}

	if (DurationProgressBar)
	{
		if (UMaterialInstanceDynamic* MID = DurationProgressBar->GetDynamicMaterial())
		{
			MID->SetScalarParameterValue(TEXT("Percentage"), 0.f);
		}
		DurationProgressBar->SetRenderOpacity(0.f);
	}

	if (Inventory001Icon) { Inventory001Icon->SetVisibility(ESlateVisibility::Hidden); }
	if (Inventory002Icon) { Inventory002Icon->SetVisibility(ESlateVisibility::Hidden); }

	Recompute(EBuffKind::Speed);
	Recompute(EBuffKind::Jump);

	SetClimbUIUpdate(0.f, 0.f);
}

void UTHPlayerHUDWidget::NativeDestruct()
{
	StopStaAnimTimer();
	GetWorld()->GetTimerManager().ClearTimer(Bars[(int)EBuffKind::Speed].BlinkTimer);
	GetWorld()->GetTimerManager().ClearTimer(Bars[(int)EBuffKind::Speed].EndHandle);
	GetWorld()->GetTimerManager().ClearTimer(Bars[(int)EBuffKind::Jump].BlinkTimer);
	GetWorld()->GetTimerManager().ClearTimer(Bars[(int)EBuffKind::Jump].EndHandle);
	GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);

	if (ActiveDebuffOverlay && ActiveDebuffOverlay->IsInViewport())
	{
		ActiveDebuffOverlay->RemoveFromParent();
		ActiveDebuffOverlay = nullptr;
	}

	UnbindAttributeDelegates();
	Super::NativeDestruct();
}


void UTHPlayerHUDWidget::BindToAbilitySystem(UAbilitySystemComponent* InASC, const UTHAttributeSet* InAttr)
{
	UnbindAttributeDelegates();
	AbilitySystem = InASC;
	Attr = InAttr;
	if (!AbilitySystem || !Attr) return;

	const float Max = FMath::Max(Attr->GetMaxStamina(), 1.f);
	StaminaInitialMax = Max;
	StaminaDisplayedPercent = Attr->GetStamina() / Max;
	if (StaminaBar) StaminaBar->SetPercent(StaminaDisplayedPercent);

	RefreshStaminaGeometry();
	Recompute(EBuffKind::Speed);
	Recompute(EBuffKind::Jump);

	BindAttributeDelegates();

	TagSprint = TAG_State_Movement_Sprinting;
	bIsSprinting = AbilitySystem->HasMatchingGameplayTag(TagSprint);
	{
		auto& Ev = AbilitySystem->RegisterGameplayTagEvent(TagSprint, EGameplayTagEventType::NewOrRemoved);
		SprintingTagHandle = Ev.AddUObject(this, &ThisClass::OnSprintingTagChanged);
	}
}

void UTHPlayerHUDWidget::BindAttributeDelegates()
{
	if (!AbilitySystem || !Attr) return;

	MaxStaminaChangedHandle =
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetMaxStaminaAttribute())
		.AddUObject(this, &ThisClass::OnMaxStaminaChanged);

	CurrentStaminaChangedHandle =
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetStaminaAttribute())
		.AddUObject(this, &ThisClass::OnCurrentStaminaChanged);

	WalkSpeedChangedHandle =
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetWalkSpeedAttribute())
		.AddUObject(this, &ThisClass::OnWalkSpeedChanged);

	SprintSpeedChangedHandle =
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetSprintSpeedAttribute())
		.AddUObject(this, &ThisClass::OnSprintSpeedChanged);

	OverlayAttrChangedHandle =
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetOverlayWidgetAttribute())
		.AddUObject(this, &ThisClass::OnOverlayWidgetAttrChanged);

}

void UTHPlayerHUDWidget::UnbindAttributeDelegates()
{
	StopStaAnimTimer();

	if (!AbilitySystem || !Attr) { AbilitySystem = nullptr; Attr = nullptr; return; }

	auto& DMaxSta = AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetMaxStaminaAttribute());
	if (MaxStaminaChangedHandle.IsValid()) { DMaxSta.Remove(MaxStaminaChangedHandle); MaxStaminaChangedHandle.Reset(); }

	auto& DCurSta = AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetStaminaAttribute());
	if (CurrentStaminaChangedHandle.IsValid()) { DCurSta.Remove(CurrentStaminaChangedHandle); CurrentStaminaChangedHandle.Reset(); }

	auto& DWalk = AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetWalkSpeedAttribute());
	if (WalkSpeedChangedHandle.IsValid()) { DWalk.Remove(WalkSpeedChangedHandle); WalkSpeedChangedHandle.Reset(); }

	auto& DSprint = AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetSprintSpeedAttribute());
	if (SprintSpeedChangedHandle.IsValid()) { DSprint.Remove(SprintSpeedChangedHandle); SprintSpeedChangedHandle.Reset(); }

	if (SprintingTagHandle.IsValid())
	{
		auto& Ev = AbilitySystem->RegisterGameplayTagEvent(TagSprint, EGameplayTagEventType::NewOrRemoved);
		Ev.Remove(SprintingTagHandle);
		SprintingTagHandle.Reset();
	}

	AbilitySystem = nullptr;
	Attr = nullptr;
}
#pragma endregion

#pragma region Stamina
// ----------------------------------- Stamina -----------------------------------------------
void UTHPlayerHUDWidget::OnMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	RefreshStaminaGeometry();

	const float Max = FMath::Max(Attr->GetMaxStamina(), 1.f);
	const float NewP = Attr->GetStamina() / Max;

	StopStaAnimTimer();
	StaminaDisplayedPercent = NewP;
	if (StaminaBar) StaminaBar->SetPercent(NewP);
}

void UTHPlayerHUDWidget::OnCurrentStaminaChanged(const FOnAttributeChangeData& Data)
{
	const float Max = FMath::Max(Attr->GetMaxStamina(), 1.f);

	const float CurrentShown = StaminaDisplayedPercent;
	const float NewP = FMath::Clamp(float(Data.NewValue) / Max, 0.f, 1.f);

	if (bIsSprinting && NewP < CurrentShown)
	{
		StaminaDisplayedPercent = NewP;
		PlayStaminaAnimFromTo(CurrentShown, NewP, StaminaAnimSpeedDown);
	}
	else if (!bIsSprinting && NewP > CurrentShown)
	{
		StaminaDisplayedPercent = NewP;
		PlayStaminaAnimFromTo(CurrentShown, NewP, StaminaAnimSpeedUp);
	}
	else
	{
		StopStaAnimTimer();
		StaminaDisplayedPercent = NewP;
		if (StaminaBar) StaminaBar->SetPercent(NewP);
	}
}

void UTHPlayerHUDWidget::PlayStaminaAnimFromTo(float From01, float To01, float SpeedPerSec)
{
	if (!StaminaBar) return;

	From01 = FMath::Clamp(From01, 0.f, 1.f);
	To01 = FMath::Clamp(To01, 0.f, 1.f);

	if (FMath::IsNearlyEqual(From01, To01, KINDA_SMALL_NUMBER))
	{
		StopStaAnimTimer();
		StaminaDisplayedPercent = To01;
		StaminaBar->SetPercent(To01);
		return;
	}

	AnimToPercent = To01;
	AnimSpeedCurrent = FMath::Max(SpeedPerSec, 0.01f);

	StaminaDisplayedPercent = From01;
	StaminaBar->SetPercent(From01);

	StartStaAnimTimer();
}

void UTHPlayerHUDWidget::StartStaAnimTimer()
{
	if (!GetWorld()) return;

	if (!bIsAnimatingStamina)
	{
		GetWorld()->GetTimerManager().SetTimer(
			StaAnimTimer,
			this,
			&ThisClass::TickStaminaAnim,
			StaminaAnimTickInterval,
			true
		);
		bIsAnimatingStamina = true;
	}
}

void UTHPlayerHUDWidget::StopStaAnimTimer()
{
	if (!GetWorld()) return;

	if (bIsAnimatingStamina)
	{
		GetWorld()->GetTimerManager().ClearTimer(StaAnimTimer);
		bIsAnimatingStamina = false;
	}
}

void UTHPlayerHUDWidget::TickStaminaAnim()
{
	if (!StaminaBar)
	{
		StopStaAnimTimer();
		return;
	}

	const float DT = GetWorld() ? GetWorld()->GetDeltaSeconds() : StaminaAnimTickInterval;
	StaminaDisplayedPercent = FMath::FInterpConstantTo(
		StaminaDisplayedPercent,
		AnimToPercent,
		DT,
		AnimSpeedCurrent
	);

	StaminaBar->SetPercent(StaminaDisplayedPercent);

	if (FMath::IsNearlyEqual(StaminaDisplayedPercent, AnimToPercent, 0.001f))
	{
		StaminaDisplayedPercent = AnimToPercent;
		StopStaAnimTimer();
	}
}

void UTHPlayerHUDWidget::RefreshStaminaGeometry()
{
	if (!StaminaBar) return;

	const float CurrentMax = FMath::Max(Attr ? Attr->GetMaxStamina() : StaminaInitialMax, 1.f);
	const float Scale = CurrentMax / FMath::Max(StaminaInitialMax, 1.f);
	const float NewWidth = StaminaBaseWidth * Scale;

	if (StaminaSizeBox)
	{
		StaminaSizeBox->SetWidthOverride(NewWidth);
	}
	else
	{
		FWidgetTransform T = StaminaBar->GetRenderTransform();
		T.Scale = FVector2D(Scale, 1.f);
		StaminaBar->SetRenderTransform(T);
	}
}

void UTHPlayerHUDWidget::OnSprintingTagChanged(FGameplayTag Tag, int32 NewCount)
{
	bIsSprinting = (NewCount > 0);
}
#pragma endregion

#pragma region Status Bars (Speed / Jump)
// ----------------------------------- General -----------------------------------------------
void UTHPlayerHUDWidget::CacheStatusBars()
{
	Bars[(int)EBuffKind::Speed].Segs.Empty();
	Bars[(int)EBuffKind::Speed].Segs.Add(SpeedBar01);
	Bars[(int)EBuffKind::Speed].Segs.Add(SpeedBar02);
	Bars[(int)EBuffKind::Speed].BaseValue = SpeedBaseValue;
	Bars[(int)EBuffKind::Speed].Step = SpeedStep;

	Bars[(int)EBuffKind::Jump].Segs.Empty();
	Bars[(int)EBuffKind::Jump].Segs.Add(JumpBar01);
	Bars[(int)EBuffKind::Jump].Segs.Add(JumpBar02);
	Bars[(int)EBuffKind::Jump].BaseValue = JumpBaseValue;
	Bars[(int)EBuffKind::Jump].Step = JumpStep;

	for (UImage* Img : Bars[(int)EBuffKind::Speed].Segs) SetSegmentOn(Img, false);
	for (UImage* Img : Bars[(int)EBuffKind::Jump].Segs) SetSegmentOn(Img, false);
}

int32 UTHPlayerHUDWidget::ComputeLevel(float Current, float Base, float Step) const
{
	if (Step <= 0.f) return 0;
	if (Current < Base)
	{
		return 0;
	}

	const float Delta = FMath::Max(0.f, Current - Base);
	const int32 Lvl = 1 + int32(Delta / Step);
	return FMath::Clamp(Lvl, 0, 2);
}

// ----------------------------------- OnChanged -----------------------------------------------
void UTHPlayerHUDWidget::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	Recompute(EBuffKind::Speed);
}

void UTHPlayerHUDWidget::OnSprintSpeedChanged(const FOnAttributeChangeData& Data)
{
	Recompute(EBuffKind::Speed);
}

void UTHPlayerHUDWidget::OnJumpAttrChanged(float NewJumpValue)
{
	auto& B = Bars[(int)EBuffKind::Jump];
	B.BaseLevel = ComputeLevel(NewJumpValue, B.BaseValue, B.Step);
	Recompute(EBuffKind::Jump);
}

void UTHPlayerHUDWidget::OnOverlayWidgetAttrChanged(const FOnAttributeChangeData& Data)
{
	//로직에 따라 오버레이 위젯을 띄우거나 제거
	if (Data.NewValue == 0)
	{
		//오버레이 제거
		RemoveDebuffOverlay();
	}
	else
	{
		//데이터 매니저에서 데이터 가져와서 배치
		const FTHItemData* OverlayData = nullptr;
		ATHItemDataManager* DM = ATHItemDataManager::Get(GetWorld());
		if (!DM) return;
		if (Data.NewValue == 1)
		{
			OverlayData = DM->GetItemDataByRow("Overlay01");
		}



		//데이터 확정 되면 오버레이 띄우기
		if (OverlayData != nullptr)
		{
			ShowFullScreenOverlay(OverlayData->VictimOverlayWidgetClass, OverlayData->DurationSec);
		}

	}
}


// ----------------------------------- Visuals -----------------------------------------------
void UTHPlayerHUDWidget::Recompute(EBuffKind Kind)
{
	auto& B = Bars[(int)Kind];

	if (Attr && Kind == EBuffKind::Speed)
	{
		const float Walk = Attr->GetWalkSpeed();
		B.BaseLevel = ComputeLevel(Walk, B.BaseValue, B.Step);
	}

	if (Attr && Kind == EBuffKind::Jump)
	{
		const float Jump = Attr->GetJumpPower();
		B.BaseLevel = ComputeLevel(Jump, B.BaseValue, B.Step);
	}

	B.FinalLevel = FMath::Clamp(B.BaseLevel + B.AddLevel, 0, 2);
	ApplyVisuals(Kind);
}

void UTHPlayerHUDWidget::ApplyVisuals(EBuffKind Kind)
{
	auto& B = Bars[(int)Kind];

	const bool L0 = (B.FinalLevel >= 1);
	const bool L1 = (B.FinalLevel >= 2);

	const bool bBlinking = (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(B.BlinkTimer));
	const int32 BlinkIdx = bBlinking ? GetTopBuffedSegIdx(B, B.BaseLevel) : INDEX_NONE;

	if (B.Segs.IsValidIndex(0) && BlinkIdx != 0) SetSegmentOn(B.Segs[0], L0);
	if (B.Segs.IsValidIndex(1) && BlinkIdx != 1) SetSegmentOn(B.Segs[1], L1);
}

void UTHPlayerHUDWidget::SetSegmentOn(UImage* Img, bool bOn)
{
	if (!Img) return;
	Img->SetRenderOpacity(bOn ? 1.f : 0.f);
	Img->SetVisibility(ESlateVisibility::Visible);
}

// ----------------------------------- Blink -----------------------------------------------
int32 UTHPlayerHUDWidget::GetTopBuffedSegIdx(const FBuffBar& B, int32 BaseLevel) const
{
	if (B.AddLevel <= 0) return INDEX_NONE;
	const int32 FirstBuffIdx = FMath::Clamp(BaseLevel, 0, 2);
	const int32 LastBuffIdx = FMath::Clamp(FirstBuffIdx + B.AddLevel - 1, 0, 1);
	return LastBuffIdx;
}

void UTHPlayerHUDWidget::BeginBlink(EBuffKind Kind, float DelaySec)
{
	if (!GetWorld()) return;
	auto& TM = GetWorld()->GetTimerManager();
	auto& B = Bars[(int)Kind];

	if (DelaySec <= 0.f)
	{
		if (!TM.IsTimerActive(B.BlinkTimer))
		{
			B.bBlinkOn = false;

			FTimerDelegate D;
			D.BindLambda([this, Kind]()
				{
					ToggleBlink(Kind);
				});
			TM.SetTimer(B.BlinkTimer, D, BlinkIntervalSec, true);
		}
		return;
	}

	FTimerHandle Tmp;
	TM.SetTimer(Tmp, [this, Kind]() { BeginBlink(Kind, 0.f); }, DelaySec, false);
}

void UTHPlayerHUDWidget::ToggleBlink(EBuffKind Kind)
{
	auto& B = Bars[(int)Kind];
	const int32 SegIdx = GetTopBuffedSegIdx(B, B.BaseLevel);
	if (!B.Segs.IsValidIndex(SegIdx)) { EndBlink(Kind); return; }

	B.bBlinkOn = !B.bBlinkOn;
	SetSegmentOn(B.Segs[SegIdx], B.bBlinkOn);
}

void UTHPlayerHUDWidget::EndBlink(EBuffKind Kind)
{
	if (!GetWorld()) return;
	auto& B = Bars[(int)Kind];
	GetWorld()->GetTimerManager().ClearTimer(B.BlinkTimer);
	B.bBlinkOn = false;
	B.AddLevel = 0;
	ApplyVisuals(Kind);
}
#pragma endregion

#pragma region Inveontory
// ----------------------------------- Inventory Icon -----------------------------------------------
void UTHPlayerHUDWidget::SetInventoryIcon(int32 SlotIndex, UTexture2D* Icon)
{
	UImage* Target = (SlotIndex == 2) ? Inventory002Icon : Inventory001Icon;
	if (!Target) return;

	if (!Icon)
	{
		Target->SetVisibility(ESlateVisibility::Hidden);
		Target->SetBrush(FSlateBrush());
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(Icon);
	Brush.ImageSize = FVector2D(80.f, 80.f);
	Target->SetBrush(Brush);
	Target->SetVisibility(ESlateVisibility::Visible);
}

// ----------------------------------- Start Duration Buff -----------------------------------------------
void UTHPlayerHUDWidget::StartSpeedDurationBuff(float DurationSec)
{
	StartDurationBuff(EBuffKind::Speed, DurationSec);
}

void UTHPlayerHUDWidget::StartJumpDurationBuff(float DurationSec)
{
	StartDurationBuff(EBuffKind::Jump, DurationSec);
}

void UTHPlayerHUDWidget::StartDurationBuff(EBuffKind Kind, float DurationSec)
{
	if (!GetWorld()) return;
	auto& TM = GetWorld()->GetTimerManager();
	auto& B = Bars[(int)Kind];

	TM.ClearTimer(B.EndHandle);
	EndBlink(Kind);

	B.AddLevel = FMath::Clamp(B.AddLevel + 1, 0, 2);
	Recompute(Kind);
	BeginBlink(Kind, FMath::Max(0.f, DurationSec - BlinkThresholdSec));

	TM.SetTimer(B.EndHandle, [this, Kind]()
		{
			auto& B2 = Bars[(int)Kind];
			B2.AddLevel = FMath::Max(0, B2.AddLevel - 1);
			Recompute(Kind);
			EndBlink(Kind);
		}, FMath::Max(0.05f, DurationSec), false);
}

// ----------------------------------- TopRightBuffIcon -----------------------------------------------
void UTHPlayerHUDWidget::ShowTopRightBuffIcon(UTexture2D* Icon, float DurationSec)
{
	if (!TopRightBuffIcon || !DurationProgressBar || !GetWorld() || !Icon) return;

	FSlateBrush B;
	B.SetResourceObject(Icon);
	B.ImageSize = FVector2D(64.f, 64.f);
	TopRightBuffIcon->SetBrush(B);
	TopRightBuffIcon->SetVisibility(ESlateVisibility::Visible);
	TopRightBuffIconBG->SetVisibility(ESlateVisibility::Visible);

	DurationTotalSec = FMath::Max(0.05f, DurationSec);
	DurationElapsedSec = 0.f;

	if (auto* MID = DurationProgressBar->GetDynamicMaterial())
		MID->SetScalarParameterValue(TEXT("Percentage"), 0.f);
	DurationProgressBar->SetRenderOpacity(1.f);

	auto& TM = GetWorld()->GetTimerManager();
	TM.ClearTimer(DurationTimerHandle);
	TM.SetTimer(DurationTimerHandle, this, &ThisClass::TickDurationTimer, 0.02f, true);
}

void UTHPlayerHUDWidget::TickDurationTimer()
{
	DurationElapsedSec += 0.02f;
	const float P = FMath::Clamp(DurationElapsedSec / FMath::Max(0.01f, DurationTotalSec), 0.f, 1.f);

	if (DurationProgressBar)
		if (auto* MID = DurationProgressBar->GetDynamicMaterial())
			MID->SetScalarParameterValue(TEXT("Percentage"), P);

	if (P >= 1.f)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);
		}
		if (TopRightBuffIcon)     TopRightBuffIcon->SetVisibility(ESlateVisibility::Hidden);
		if (TopRightBuffIconBG)	  TopRightBuffIconBG->SetVisibility(ESlateVisibility::Hidden);
		if (DurationProgressBar)  DurationProgressBar->SetRenderOpacity(0.f);
	}
}

void UTHPlayerHUDWidget::StopDurationTimer()
{
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);
	if (DurationProgressBar)
		DurationProgressBar->SetRenderOpacity(0.f);
}

// ----------------------------------- Full Screen Overlay -----------------------------------------------
void UTHPlayerHUDWidget::ShowFullScreenOverlay(TSubclassOf<UUserWidget> OverlayClass, float DurationSec)
{
	if (!OverlayClass || !GetWorld()) return;
	if (ActiveDebuffOverlay && ActiveDebuffOverlay->IsInViewport())
	{
		ActiveDebuffOverlay->RemoveFromParent();
		ActiveDebuffOverlay = nullptr;
	}

	ActiveDebuffOverlay = CreateWidget<UUserWidget>(GetWorld(), OverlayClass);
	if (ActiveDebuffOverlay)
	{
		ActiveDebuffOverlay->AddToViewport(INT_MAX);
		//FTimerHandle Tmp;		
		GetWorld()->GetTimerManager().SetTimer(/*Tmp*/DebuffOverlayTimerHandle, [this]()
			{
				if (ActiveDebuffOverlay)
				{
					ActiveDebuffOverlay->RemoveFromParent();
					ActiveDebuffOverlay = nullptr;
				}
			}, FMath::Max(0.05f, DurationSec), false);
	}
}


void UTHPlayerHUDWidget::RemoveDebuffOverlay()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(DebuffOverlayTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(DebuffOverlayTimerHandle);
	}
	if (ActiveDebuffOverlay)
	{
		ActiveDebuffOverlay->RemoveFromParent();
		ActiveDebuffOverlay = nullptr;
	}
}


#pragma endregion

#pragma region Climbing&Rank
void UTHPlayerHUDWidget::SetRankUIUpdate(bool bBunnyWinning)
{
	if (bBunnyWinning && !bHasBunnyBeenWinning)
	{
		PlayAnimation(RabbitUpAnim);
		bHasBunnyBeenWinning = true;
	}
	else if (!bBunnyWinning && bHasBunnyBeenWinning)
	{
		PlayAnimation(RabbitUpAnim, 0.f, 1, EUMGSequencePlayMode::Reverse);
		bHasBunnyBeenWinning = false;
	}
}

void UTHPlayerHUDWidget::SetClimbUIUpdate(float SelfP, float OppoP)
{
	TargetSelfP = FMath::Clamp(SelfP, 0.f, 1.f);
	TargetOppoP = FMath::Clamp(OppoP, 0.f, 1.f);

	if (ClimbingBar) SetClimbSelfUpdate(SelfP);
	if (OppositeClimbPoint) SetClimbOppoUpdate(OppoP);

	auto& TM = GetWorld()->GetTimerManager();
	if (!TM.IsTimerActive(ClimbSmoothTimer))
	{
		TM.SetTimer(ClimbSmoothTimer, this, &ThisClass::ClimbSmoothing, 0.02f, true);
	}
}

void UTHPlayerHUDWidget::SetClimbSelfUpdate(float SelfP)
{
	SelfP = FMath::Clamp(SelfP, 0.f, 1.f);
	ClimbingBar->SetPercent(SelfP);
}

void UTHPlayerHUDWidget::SetClimbOppoUpdate(float OppoP)
{
	OppoP = FMath::Clamp(OppoP, 0.f, 1.f);
	if (!bOppoBaseYInit)
	{
		OppoBaseY = OppositeClimbPoint->GetRenderTransform().Translation.Y;
		bOppoBaseYInit = true;
	}

	const float NewY = OppoBaseY + (OppoTravelDeltaY * OppoP);

	FWidgetTransform T = OppositeClimbPoint->GetRenderTransform();
	T.Translation.Y = NewY;
	OppositeClimbPoint->SetRenderTransform(T);
}

void UTHPlayerHUDWidget::ClimbSmoothing()
{
	const float dt = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.02f;

	DisplayedSelfP = FMath::FInterpTo(DisplayedSelfP, TargetSelfP, dt, 3.f);
	DisplayedOppoP = FMath::FInterpTo(DisplayedOppoP, TargetOppoP, dt, 3.f);

	if (ClimbingBar) SetClimbSelfUpdate(DisplayedSelfP);
	if (OppositeClimbPoint) SetClimbOppoUpdate(DisplayedOppoP);

	if (FMath::IsNearlyEqual(DisplayedSelfP, TargetSelfP, 0.001f) &&
		FMath::IsNearlyEqual(DisplayedOppoP, TargetOppoP, 0.001f))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClimbSmoothTimer);
	}
}
#pragma endregion