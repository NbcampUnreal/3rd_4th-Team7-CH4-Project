// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "THLoadingWidget.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class TREASUREHUNTER_API UTHLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void LoadProgressState();
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* LoadingProgressBar;

private:
	FTimerHandle LoadingTimerHandle;

	float Accumulate;
	float LoadTime = 0.01f;
	float TotalLoadTime = 10.0f;
	
private:
	void UpdateProgress();

	UFUNCTION()
	void OnStreamableCompleted();
};
