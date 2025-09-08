// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THLoadingWidget.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

void UTHLoadingWidget::StartLoadLevel(TSoftObjectPtr<UWorld> LevelToLoad)
{
	TargetLevel = LevelToLoad;
	if (!TargetLevel.IsValid() || !TargetLevel.ToSoftObjectPath().IsValid()) return;
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(0.f);
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	StreamHandle = Streamable.RequestAsyncLoad(
		TargetLevel.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UTHLoadingWidget::OnStreamableCompleted),
		FStreamableManager::DefaultAsyncLoadPriority,
		false
	);
	
	if (StreamHandle.IsValid())
	{
		StreamHandle->BindUpdateDelegate(
			FStreamableUpdateDelegate::CreateUObject(this, &UTHLoadingWidget::OnStreamableUpdate)
		);
	}
}

void UTHLoadingWidget::OnStreamableUpdate(TSharedRef<FStreamableHandle> InHandle)
{
	if (!StreamHandle.IsValid() || !LoadingProgressBar) return;

	const float Percent = FMath::Clamp(StreamHandle->GetProgress(), 0.f, 1.f);
	LoadingProgressBar->SetPercent(Percent);
}

void UTHLoadingWidget::OnStreamableCompleted()
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(1.f);
	}

	if (TargetLevel.IsValid())
	{
		// if Singleplayer game,
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, TargetLevel, true);
		// if Multiplayer game,
		// GetOwningPlayer()->ClientTravel(TargetLevel.ToSoftObjectPath().ToString(), TRAVEL_Absolute);
	}
	StreamHandle.Reset();
}
