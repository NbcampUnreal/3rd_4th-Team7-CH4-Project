// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "THGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChangedSig, FGameplayTag, NewPhase);

UCLASS()
class TREASUREHUNTER_API ATHGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(ReplicatedUsing = OnRep_PhaseTag, BlueprintReadOnly, Category = "GameState")
	FGameplayTag PhaseTag;

	UPROPERTY(BlueprintAssignable)
	FOnPhaseChangedSig OnPhaseChanged;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	FGameplayTag WinnerTag;

	UFUNCTION(BlueprintCallable)
	void SetPhase(const FGameplayTag& NewPhase);

	UFUNCTION() 
	void OnRep_PhaseTag();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
};
