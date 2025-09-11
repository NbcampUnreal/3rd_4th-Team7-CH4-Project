#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "THInteractPromptWidget.h"
#include "THBaseItem.generated.h"

class UStaticMeshComponent;
class ATHPlayerCharacter;

UCLASS()
class TREASUREHUNTER_API ATHBaseItem : public AActor
{
	GENERATED_BODY()
	
public:
	ATHBaseItem();

protected:
	virtual void BeginPlay() override;


public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FString ItemID;

	void SetItemID(FString NewItemID);


	virtual void Tick(float DeltaTime) override;

	//아이템 매쉬가 있고 상호작용하여 획득(인벤토리에 이전 이 후 삭제)




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


	void ItemPickup(ATHPlayerCharacter* PlayerCharacter);

	bool bIsPickedUp;
	FTimerHandle PickedUpTimerHandle;
	void PickedUpTime();
};
