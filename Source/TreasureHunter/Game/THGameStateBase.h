// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "THGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChangedSig, FGameplayTag, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRematchChangedSig, FGameplayTag, NewTag);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotsUpdatedSig);

class APlayerState;

UCLASS()
class TREASUREHUNTER_API ATHGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

#pragma region Phase
public:
	UPROPERTY(ReplicatedUsing = OnRep_PhaseTag, BlueprintReadOnly, Category = "GameState")
	FGameplayTag PhaseTag;

	UPROPERTY(BlueprintAssignable)
	FOnPhaseChangedSig OnPhaseChanged;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	FGameplayTag WinnerTag;

public:
	UFUNCTION(BlueprintCallable)
	void SetPhase(const FGameplayTag& NewPhase);

	UFUNCTION(BlueprintCallable)
	void SetWinnerTag(const FGameplayTag& NewWinner);

private:
	UFUNCTION()
	void OnRep_PhaseTag();
#pragma endregion

#pragma region Matchmaking
public:
	UFUNCTION()
	void OnRep_SlotOwners();

	UFUNCTION()
	void OnRep_SlotsLockedIn();

public:
	UPROPERTY(ReplicatedUsing = OnRep_SlotOwners, BlueprintReadOnly, Category = "Matchmaking")
	TArray<TObjectPtr<APlayerState>> SlotOwners;

	// Both Players Ready 
	UPROPERTY(ReplicatedUsing = OnRep_SlotsLockedIn, BlueprintReadOnly, Category = "Matchmaking")
	bool bSlotsLockedIn = false;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking")
	FOnSlotsUpdatedSig OnSlotsUpdated;

public:
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	APlayerState* GetSlotOwner(int32 SlotIdx) const;

	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool AreSlotsFilled() const;

	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool AreBothReady() const;

	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	FORCEINLINE bool AreSlotsLockedIn() const { return bSlotsLockedIn; }

	bool TryAssignSlot(int32 SlotIdx, APlayerState* Requestor);

	void ResetSlots();

#pragma endregion

#pragma region Rematch
public:
	UFUNCTION()
	void OnRep_RematchTag();

	UFUNCTION(BlueprintCallable, Category = "Rematch")
	APlayerState* GetRematchRequester() const { return RematchRequester; }

	UFUNCTION(BlueprintCallable, Category = "Rematch")
	APlayerState* GetRematchResponder() const { return RematchResponder; }

	UFUNCTION(BlueprintCallable, Category = "Rematch")
	uint8 GetRematchAcceptMask() const { return RematchAcceptMask; }

	UFUNCTION(BlueprintCallable, Category = "Rematch")
	float GetRematchExpireAt() const { return RematchExpireAt; }

	UFUNCTION(BlueprintCallable, Category = "Rematch")
	void ResetRematchState();

public:
	UPROPERTY(BlueprintAssignable)
	FOnRematchChangedSig OnRematchChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RematchTag, BlueprintReadOnly, Category = "Rematch")
	FGameplayTag RematchTag;

private:
	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> RematchRequester = nullptr;

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> RematchResponder = nullptr;

	UPROPERTY(Replicated)
	uint8 RematchAcceptMask = 0; // 0=Requester, 1=Responder

	UPROPERTY(Replicated)
	float RematchExpireAt = 0.f;
#pragma endregion
};
