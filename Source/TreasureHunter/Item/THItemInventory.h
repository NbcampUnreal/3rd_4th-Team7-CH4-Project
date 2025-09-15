#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "THItemInventory.generated.h"



class ATHItemDataManager;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotChanged, int32, SlotIndex, FName, ItemID);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TREASUREHUNTER_API UTHItemInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	UTHItemInventory();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(ReplicatedUsing = OnRep_ItemSlot1)
	FName ItemSlot1;
	UPROPERTY(ReplicatedUsing = OnRep_ItemSlot2)
	FName ItemSlot2;

	/*UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddItem(FName NewItemID);*/

	bool AddItem(FName NewItemID);

	UFUNCTION(Server, Reliable)
	void Server_UseItem(int32 SlotIndex);

	void UseItem(int32 SlotIndex);

	bool UseTimeCheck = false;

	FTimerHandle UseTimerHandle;
	void ResetUseTime();

	
	UPROPERTY()
	TObjectPtr<ATHItemDataManager> ItemDataManager;


	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventorySlotChanged OnInventorySlotChanged;

	UFUNCTION()
	void OnRep_ItemSlot1(FName OldValue);

	UFUNCTION()
	void OnRep_ItemSlot2(FName OldValue);

	UFUNCTION(BlueprintPure)
	FName GetItemInSlot(int32 SlotIndex) const;

private:
	static void HandleSlotRep(UTHItemInventory* Self, int32 SlotIdx, FName OldVal, FName NewVal);
};
