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
	void StartLoadLevel(TSoftObjectPtr<UWorld> LevelToLoad);
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* LoadingProgressBar;

private:
	TSharedPtr<FStreamableHandle> StreamHandle;
	TSoftObjectPtr<UWorld> TargetLevel;
	
private:
	void OnStreamableUpdate(TSharedRef<FStreamableHandle> InHandle);
	void OnStreamableCompleted();
};
