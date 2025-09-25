// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "THStompStunCue.generated.h"

/**
 * 
 */
UCLASS()
class TREASUREHUNTER_API UTHStompStunCue : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	TSubclassOf<class UCameraShakeBase> ShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float ShakeScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	FName StunEffectSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect")
	class UNiagaraSystem* StompEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundBase* StompSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	UAnimMontage* StunAnimation;
};
