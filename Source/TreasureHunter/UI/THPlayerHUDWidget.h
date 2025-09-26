// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "THPlayerHUDWidget.generated.h"

class UImage;
class UProgressBar;
class UHorizontalBox;
class USizeBox;
class UTextBlock;
class UWidgetAnimation;
class UAbilitySystemComponent;
class UTHAttributeSet;
class UUserWidget;
struct FOnAttributeChangeData;

UENUM()
enum class EBuffKind : uint8
{
	Speed = 0,
	Jump = 1
};

USTRUCT()
struct FBuffBar
{
	GENERATED_BODY()

	UPROPERTY() TArray<TObjectPtr<UImage>> Segs;
	int32  BaseLevel = 1;
	int32  AddLevel = 0;
	int32  FinalLevel = 0;

	float  BaseValue = 0.f; // Speed: SpeedBaseValue, Jump: JumpBaseValue
	float  Step = 0.f; // Speed: SpeedStep, Jump: JumpStep

	FTimerHandle BlinkTimer;
	FTimerHandle EndHandle;
	bool bBlinkOn = false;
};

UCLASS()
class TREASUREHUNTER_API UTHPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTHPlayerHUDWidget(const FObjectInitializer& ObjectInitializer);

	void BindToAbilitySystem(UAbilitySystemComponent* AbilitySystem, const UTHAttributeSet* InAttr);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindAttributeDelegates();
	void UnbindAttributeDelegates();

#pragma region Stamina

private:
	UFUNCTION()
	void OnSprintingTagChanged(FGameplayTag Tag, int32 NewCount);

	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);
	void OnCurrentStaminaChanged(const FOnAttributeChangeData& Data);
	void RefreshStaminaGeometry();

	void PlayStaminaAnimFromTo(float From01, float To01, float SpeedPerSec);
	void StartStaAnimTimer();
	void StopStaAnimTimer();
	void TickStaminaAnim();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	USizeBox* StaminaSizeBox;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UProgressBar* StaminaBar;

private:
	UPROPERTY(EditAnywhere, Category = "HUD|Stamina")
	float StaminaBaseWidth;
	float StaminaInitialMax;
	float StaminaDisplayedPercent;

	UPROPERTY(EditAnywhere, Category = "HUD|Stamina", meta = (ClampMin = "0.005", ClampMax = "0.1"))
	float StaminaAnimTickInterval;

	UPROPERTY(EditAnywhere, Category = "HUD|Stamina", meta = (ClampMin = "0.01"))
	float StaminaAnimSpeedDown;

	UPROPERTY(EditAnywhere, Category = "HUD|Stamina", meta = (ClampMin = "0.01"))
	float StaminaAnimSpeedUp;

	FTimerHandle StaAnimTimer;
	bool bIsAnimatingStamina;
	float AnimToPercent;
	float AnimSpeedCurrent;

	bool bIsSprinting;

#pragma endregion

#pragma region Status Bars (Speed / Jump)
public:
	UFUNCTION(BlueprintCallable, Category = "HUD|Stat")
	void StartSpeedDurationBuff(float DurationSec);
	UFUNCTION(BlueprintCallable, Category = "HUD|Stat")
	void StartJumpDurationBuff(float DurationSec);

private:
	void StartDurationBuff(EBuffKind Kind, float DurationSec);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UHorizontalBox* StatusBar;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) UImage* SpeedBar01;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) UImage* SpeedBar02;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) UImage* JumpBar01;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) UImage* JumpBar02;

private:
	FBuffBar Bars[2];

	UPROPERTY(EditAnywhere, Category = "HUD|Stat|Speed")
	float SpeedBaseValue;
	UPROPERTY(EditAnywhere, Category = "HUD|Stat|Speed")
	float SpeedStep;
	UPROPERTY(EditAnywhere, Category = "HUD|Stat|Jump")
	float JumpBaseValue;
	UPROPERTY(EditAnywhere, Category = "HUD|Stat|Jump")
	float JumpStep;

	UPROPERTY(EditAnywhere, Category = "HUD|Stat|Buff")
	float BlinkThresholdSec = 3.f;
	UPROPERTY(EditAnywhere, Category = "HUD|Stat|Buff")
	float BlinkIntervalSec = 0.2f;

	void CacheStatusBars();

	void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	void OnSprintSpeedChanged(const FOnAttributeChangeData& Data);
	void OnJumpAttrChanged(float NewJumpValue);
	void OnOverlayWidgetAttrChanged(const FOnAttributeChangeData& Data);

	FTimerHandle DebuffOverlayTimerHandle;

	int32 ComputeLevel(float Current, float Base, float Step) const; // 0~2
	void Recompute(EBuffKind Kind);
	void ApplyVisuals(EBuffKind Kind);
	void SetSegmentOn(UImage* Img, bool bOn);

	int32 GetTopBuffedSegIdx(const FBuffBar& B, int32 BaseLevel) const;
	void BeginBlink(EBuffKind Kind, float DelaySec);
	void ToggleBlink(EBuffKind Kind);
	void EndBlink(EBuffKind Kind);
#pragma endregion

#pragma region Inventory
public:
	UFUNCTION(BlueprintCallable, Category = "HUD|Item")
	void SetInventoryIcon(int32 SlotIndex, UTexture2D* Icon);

	UFUNCTION(BlueprintCallable, Category = "HUD|Item")
	void ShowTopRightBuffIcon(UTexture2D* Icon, float DurationSec);

	UFUNCTION(BlueprintCallable, Category = "HUD|Item")
	void ShowFullScreenOverlay(TSubclassOf<UUserWidget> OverlayClass, float DurationSec);

	void RemoveDebuffOverlay();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* DurationProgressBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* TopRightBuffIcon;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* TopRightBuffIconBG;

	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* Inventory001Icon;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* Inventory002Icon;

private:
	FTimerHandle DurationTimerHandle;
	float DurationTotalSec;
	float DurationElapsedSec;

	UPROPERTY() 
	UUserWidget* ActiveDebuffOverlay = nullptr;

	void TickDurationTimer();
	void StopDurationTimer();
#pragma endregion

#pragma region Climbing&Rank
protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UProgressBar* ClimbingBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* OppositeClimbPoint;
	UPROPERTY(Transient, BlueprintReadWrite, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* RabbitUpAnim;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BunnyRank;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MouseRank;

private:
	bool bHasBunnyBeenWinning;
	bool  bOppoBaseYInit = false;
	float OppoBaseY;
	float OppoTravelDeltaY;

	FTimerHandle ClimbSmoothTimer;
	float TargetSelfP;
	float TargetOppoP;
	float DisplayedSelfP, DisplayedOppoP;

public:
	void SetRankUIUpdate(bool bBunnyWinning);
	void SetClimbUIUpdate(float SlefP, float OppoP);

private:
	void SetClimbSelfUpdate(float SelfP);
	void SetClimbOppoUpdate(float OppoP);

	void ClimbSmoothing();
#pragma endregion

private:
	UPROPERTY()
	const UTHAttributeSet* Attr;
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystem;
	FGameplayTag TagSprint;

	FDelegateHandle MaxStaminaChangedHandle;
	FDelegateHandle CurrentStaminaChangedHandle;
	FDelegateHandle WalkSpeedChangedHandle;
	FDelegateHandle SprintSpeedChangedHandle;
	FDelegateHandle SprintingTagHandle;
	FDelegateHandle OverlayAttrChangedHandle;
};