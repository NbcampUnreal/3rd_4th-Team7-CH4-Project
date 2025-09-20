#pragma once

#include "GameFramework/Actor.h"
#include "Item/Item_Data/THItemData.h"
#include "THInteractPromptWidget.h"
#include "THItemBox.generated.h"

class USphereComponent; // 중복선언말고 전방선언



UCLASS()
class TREASUREHUNTER_API ATHItemBox : public AActor
{
	GENERATED_BODY()
	
public:
	ATHItemBox();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	int32 DropHeight;

	
	UFUNCTION(BlueprintCallable, Category = "ItemBox")
	void OpenBox();

	
	FName RandomItemGenerate(EItemType DropType);
	void DropItem(FName RandomItemID);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* OverlapSphere;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTHInteractPromptWidget> InteractPromptClass;

	UPROPERTY()
	UTHInteractPromptWidget* InteractPromptWidget;

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	bool UseTimeCheck = false;

	FTimerHandle UseTimerHandle;
	void ResetUseTime();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DestroyBox();
	
};
