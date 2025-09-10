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

protected:
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

#pragma region Speed

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetSpeedLevel(int32 InLevel);

private:
	void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	void OnSprintSpeedChanged(const FOnAttributeChangeData& Data);

	void RefreshSpeedLevel();
	void RebuildSpeedSegmentsCache();
	void ApplySpeedLevelVisuals();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UHorizontalBox* SpeedBar;

private:
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

#pragma endregion

#pragma region Inventory
public:
	UFUNCTION(BlueprintCallable, Category = "HUD|Item")
	void SetInventoryIcon(int32 SlotIndex, UTexture2D* Icon);
	UFUNCTION(BlueprintCallable, Category = "HUD|Item")
	void ClearInventoryIcon(int32 SlotIndex, float CoolTime);

private:
	UFUNCTION(Category = "HUD|Item")
	void StartCoolTimeTimer(float Duration);

	void UpdateCoolTime();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* CoolTimeProgressBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* Inventory001Icon;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* Inventory002Icon;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	UImage* InventoryCoolTimeIcon;

private:
	FTimerHandle CoolTimeTimer;
	float CoolTimeDuration;
	float CoolTimeElapsed;

#pragma endregion



protected:
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

};