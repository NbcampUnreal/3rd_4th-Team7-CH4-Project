// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "THFootStepCue.generated.h"

/**
 * 
 */
UCLASS()
class TREASUREHUNTER_API UTHFootStepCue : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundBase* FootStepSound;
	
};
