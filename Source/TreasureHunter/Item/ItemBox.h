#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemData.h"
#include "Components/SphereComponent.h"
#include "InteractPromptWidget.h"
#include "ItemBox.generated.h"


UCLASS()
class TREASUREHUNTER_API AItemBox : public AActor
{
	GENERATED_BODY()
	
public:
	AItemBox();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	int32 DropHeight;

	void OpenBox();
	FString RandomItemGenerate(EItemType DropType);
	void DropItem(FString RandomItemID);

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
	TSubclassOf<UInteractPromptWidget> InteractPromptClass;

	UPROPERTY()
	UInteractPromptWidget* InteractPromptWidget;

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

	
};
