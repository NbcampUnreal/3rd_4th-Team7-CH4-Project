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

	FTimerHandle TurningTimerHandle;
	void TurningItem();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName ItemID;

	void SetItemID(FName NewItemID);



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* OverlapSphere;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTHInteractPromptWidget> InteractPromptClass;

	UPROPERTY()
	UTHInteractPromptWidget* InteractPromptWidget;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	bool ItemPickup(ATHPlayerCharacter* PlayerCharacter);

	UPROPERTY(Replicated)
	bool bIsPickedUp;
	FTimerHandle PickedUpTimerHandle;
	void EnablePickup();



	UPROPERTY()
	TMap<APlayerController*, UTHInteractPromptWidget*> InteractPromptWidgets;

	UPROPERTY(EditAnywhere, Category = "UI")
	float RotationSpeed = 1.f;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPickupEffect();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundBase* EffectSound;
};
