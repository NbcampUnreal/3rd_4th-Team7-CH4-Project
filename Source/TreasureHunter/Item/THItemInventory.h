#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "THItemInventory.generated.h"

class ATHItemDataManager;
class UTexture2D;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TREASUREHUNTER_API UTHItemInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	UTHItemInventory();

protected:
	virtual void BeginPlay() override;

public:
	//인벤토리 어떻게 할지 고민해봐야 함

	//현재 슬롯이 2개인데, 3번째 획득 아이템에 대해 어떻게 처리할 것인가?

	//1. 획득 불가 2. 기존 아이템 버리고 획득 3. 기존 아이템과 교환

	//일단 1번을 가정하고 제작

	//아이템 슬롯 2칸을 관리하여 빈칸을 찾아 아이템 획득
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FString ItemSlot1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FString ItemSlot2;

	//아이템 획득 함수
	bool AddItem(FString NewItemID);

	void UseItem(int32 SlotIndex);

	bool UseTimeCheck = false;

	FTimerHandle UseTimerHandle;
	void ResetUseTime();

	void UpdateItemImage(int32 SlotIndex, const FString& ItemID);

	UPROPERTY()
	TObjectPtr<ATHItemDataManager> ItemDataManager;
};
