#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "THItemInventory.generated.h"



class ATHItemDataManager;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotChanged, int32, SlotIndex, FName, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemActivated, int32, SlotIndex, FName, ItemID);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TREASUREHUNTER_API UTHItemInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	UTHItemInventory();

	bool AddItem(FName NewItemRow);

	UFUNCTION(Server, Reliable)
	void Server_UseItem(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FName GetItemInSlot(int32 SlotIndex) const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventorySlotChanged OnInventorySlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryItemActivated OnItemActivated;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_ItemSlot1)
	FName ItemSlot1 = NAME_None;
	UPROPERTY(ReplicatedUsing = OnRep_ItemSlot2)
	FName ItemSlot2 = NAME_None;

	UFUNCTION()
	void OnRep_ItemSlot1();

	UFUNCTION()
	void OnRep_ItemSlot2();

	void UseItem(int32 SlotIndex);

	UFUNCTION(Client, Reliable)
	void Client_NotifyItemActivated(FName ItemRow, int32 SlotIndex);
	
private:
	UPROPERTY()
	TObjectPtr<ATHItemDataManager> ItemDataManager;

	bool bUseTimeCheck = false;

	FTimerHandle UseTimerHandle;
	void ResetUseTime();

public:
	UFUNCTION(Client, Reliable)
	void Client_PlayItemUseSound();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundBase* EffectSound;
};
