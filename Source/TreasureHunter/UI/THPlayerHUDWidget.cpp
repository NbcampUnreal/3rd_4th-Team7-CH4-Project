// Fill out your copyright notice in the Description page of Project Settings.


#include "THPlayerHUDWidget.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet/THAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "TimerManager.h"
#include "Game/GameFlowTags.h"

UTHPlayerHUDWidget::UTHPlayerHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Attr(nullptr)
	, AbilitySystem(nullptr)
	, StaminaBaseWidth(500.f)
	, StaminaInitialMax(100.f)
	, StaminaDisplayedPercent(1.f)
	, StaminaAnimTickInterval(0.02f)
	, StaminaAnimSpeedDown(2.0f)
	, StaminaAnimSpeedUp(1.0f)
	, bIsAnimatingStamina(false)
	, AnimToPercent(1.f)
	, AnimSpeedCurrent(1.f)
	, SpeedLevel(1)
	, SpeedMaxLevel(5)
	, SpeedStep(50.f)
	, SpeedActiveImage(nullptr)
	, SpeedInactiveImage(nullptr)
	, bIsSprinting(false)
{
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
	RebuildSpeedSegmentsCache();
	RefreshSpeedLevel();
	ApplySpeedLevelVisuals();

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

void UTHPlayerHUDWidget::NativeDestruct()
{
	StopStaAnimTimer();
	UnbindAttributeDelegates();
	Super::NativeDestruct();
}

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
		FWidgetTransform T = StaminaBar->RenderTransform;
		T.Scale = FVector2D(Scale, 1.f);
		StaminaBar->SetRenderTransform(T);
	}
}

void UTHPlayerHUDWidget::OnSprintingTagChanged(FGameplayTag Tag, int32 NewCount)
{
	bIsSprinting = (NewCount > 0);
}
#pragma endregion

#pragma region Speed
// ----------------------------------- Speed -----------------------------------------------
void UTHPlayerHUDWidget::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	RefreshSpeedLevel();
	ApplySpeedLevelVisuals();
}

void UTHPlayerHUDWidget::OnSprintSpeedChanged(const FOnAttributeChangeData& Data)
{
	RefreshSpeedLevel();
	ApplySpeedLevelVisuals();
}

void UTHPlayerHUDWidget::RefreshSpeedLevel()
{
	if (!Attr) return;
	const float Walk = Attr->GetWalkSpeed();
	const int32 Delta = FMath::Max(0, Walk - 300);
	SpeedLevel = FMath::Clamp(1 + (Delta / SpeedStep), 1, SpeedMaxLevel);
}

void UTHPlayerHUDWidget::SetSpeedLevel(int32 InLevel)
{
	SpeedLevel = FMath::Clamp(InLevel, 1, SpeedMaxLevel);
	ApplySpeedLevelVisuals();
}

void UTHPlayerHUDWidget::RebuildSpeedSegmentsCache()
{
	SpeedSegments.Reset();

	if (!SpeedBar) return;

	const int32 N = SpeedBar->GetChildrenCount();
	for (int32 i = 0; i < N; ++i)
	{
		if (UWidget* Child = SpeedBar->GetChildAt(i))
		{
			if (UImage* Img = Cast<UImage>(Child))
			{
				SpeedSegments.Add(Img);
			}
		}
	}
}

void UTHPlayerHUDWidget::ApplySpeedLevelVisuals()
{
	const int32 NumSeg = SpeedSegments.Num();
	if (NumSeg == 0) return;

	const int32 VisibleActive = FMath::Clamp(SpeedLevel, 0, NumSeg);

	for (int32 i = 0; i < NumSeg; ++i)
	{
		UImage* Img = SpeedSegments[i];
		if (!Img) continue;

		const bool bActive = (i < VisibleActive);
		UTexture2D* Tex = bActive ? SpeedActiveImage : SpeedInactiveImage;

		if (Tex)
		{
			Img->SetBrushFromTexture(Tex, true);
		}

		FSlateBrush Brush = Img->Brush;
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.ImageSize = FVector2D(70.f, 40.f);
		Img->SetBrush(Brush);
	}
}
#pragma endregion

#pragma region Inveontory

void UTHPlayerHUDWidget::SetInventoryIcon(int32 SlotIndex, UTexture2D* Icon)
{
	if (!Icon) return;

	FSlateBrush Brush;
	Brush.SetResourceObject(Icon);
	Brush.ImageSize = FVector2D(80.f, 80.f);
	(SlotIndex == 1) ? Inventory001Icon->SetBrush(Brush) : Inventory002Icon->SetBrush(Brush);
}

void UTHPlayerHUDWidget::ClearInventoryIcon(int32 SlotIndex, float CoolTime)
{
	UImage* TargetIcon = nullptr;
	(SlotIndex == 1) ? TargetIcon = Inventory001Icon : TargetIcon = Inventory002Icon;

	if (TargetIcon)
	{
		if (InventoryCoolTimeIcon)
		{
			InventoryCoolTimeIcon->SetBrush(TargetIcon->Brush);
			TargetIcon->SetBrush(FSlateBrush());
		}
		StartCoolTimeTimer(CoolTime);
	}
}

void UTHPlayerHUDWidget::StartCoolTimeTimer(float Duration)
{
	if (!CoolTimeProgressBar) return;
	
	(Duration <= 0.5f) ? CoolTimeDuration = 0.5f : CoolTimeDuration = Duration;
	CoolTimeElapsed = 0.f;

	CoolTimeProgressBar->SetRenderOpacity(1.f);
	GetWorld()->GetTimerManager().SetTimer(
		CoolTimeTimer, this, &UTHPlayerHUDWidget::UpdateCoolTime, 0.02f, true
	);
}

void UTHPlayerHUDWidget::UpdateCoolTime()
{
	CoolTimeElapsed += 0.02f;
	float Percent = FMath::Clamp(CoolTimeElapsed / CoolTimeDuration, 0.f, 1.f);

	if (CoolTimeProgressBar)
	{
		CoolTimeProgressBar->GetDynamicMaterial()->SetScalarParameterValue(TEXT("Percentage"), Percent);
	}

	if (Percent >= 1.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CoolTimeTimer);
		if (InventoryCoolTimeIcon)
		{
			InventoryCoolTimeIcon->SetBrush(FSlateBrush());
		}

		CoolTimeProgressBar->SetRenderOpacity(0.f);
	}
}

#pragma endregion