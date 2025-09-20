#pragma once

#include "GameFramework/Actor.h"
#include "THBaseItem.generated.h"

class USphereComponent; // 최대한 전방선언으로 의존성 떨어뜨리기 ! 불필요한 컴파일을 줄이고 메모리 사용 효율 높여줌 
class UTHInteractPromptWidget;
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


};
