// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/THLoadingWidget.h"
#include "Components/ProgressBar.h"
#include "Player/Controller/THTitlePlayerController.h"
#include "Kismet/GameplayStatics.h"

void UTHLoadingWidget::LoadProgressState()
{
	if (!LoadingProgressBar) return;

	LoadingProgressBar->SetPercent(0.f);
	Accumulate = 0.f;
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("OnStreamableCompleted"));
	GetWorld()->GetTimerManager().SetTimer(
		LoadingTimerHandle,
		this,
		&UTHLoadingWidget::UpdateProgress,
		LoadTime,
		true);
}

void UTHLoadingWidget::UpdateProgress()
{
	Accumulate += LoadTime;

	float Percent = FMath::Clamp(Accumulate / TotalLoadTime, 0.f, 1.f);
	LoadingProgressBar->SetPercent(Percent);

	if (Percent >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LoadingTimerHandle);
		OnStreamableCompleted();
	}
}

void UTHLoadingWidget::OnStreamableCompleted()
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(1.f);
	}

	ATHTitlePlayerController* PC = GetOwningPlayer<ATHTitlePlayerController>();
	if (PC)
	{
		PC->OpenPlayLevel();
	}
}
