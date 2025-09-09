// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "THPlayerHUDWidget.generated.h"

class UImage;
class UProgressBar;
class UHorizontalBox;
class USizeBox;
class UAbilitySystemComponent;
class UTHAttributeSet;
struct FOnAttributeChangeData;

UCLASS()
class TREASUREHUNTER_API UTHPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTHPlayerHUDWidget(const FObjectInitializer& ObjectInitializer);

	void BindToAbilitySystem(UAbilitySystemComponent* AbilitySystem, const UTHAttributeSet* InAttr);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetSpeedLevel(int32 InLevel);

protected:
	virtual void NativeDestruct() override;

private:
	void BindAttributeDelegates();
	void UnbindAttributeDelegates();

	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);
	void OnCurrentStaminaChanged(const FOnAttributeChangeData& Data);
	void RefreshStaminaGeometry();

	void PlayStaminaAnimFromTo(float From01, float To01, float SpeedPerSec);
	void StartStaAnimTimer();
	void StopStaAnimTimer();
	void TickStaminaAnim();

	UFUNCTION()
	void OnSprintingTagChanged(FGameplayTag Tag, int32 NewCount);
	void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	void OnSprintSpeedChanged(const FOnAttributeChangeData& Data);

	void RefreshSpeedLevel();
	void RebuildSpeedSegmentsCache();
	void ApplySpeedLevelVisuals();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* CoolTimeProgressBar;

	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UHorizontalBox* SpeedBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	USizeBox* StaminaSizeBox;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UProgressBar* StaminaBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UProgressBar* ClimbingBar;

	UPROPERTY(meta = (BindWidget))
	UImage* SecondPlayerIMG;
	UPROPERTY(meta = (BindWidget))
	UImage* FirstPlayerIMG;

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

	int32  SpeedLevel;
	UPROPERTY(EditAnywhere, Category = "HUD|Speed")
	int32  SpeedMaxLevel;
	float SpeedStep;

	UPROPERTY() 
	TArray<UImage*> SpeedSegments;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Speed")
	UTexture2D* SpeedActiveImage;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Speed")
	UTexture2D* SpeedInactiveImage;

	bool bIsSprinting;
};