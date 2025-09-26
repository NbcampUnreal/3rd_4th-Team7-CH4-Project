#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "THItemData.h"
#include "Components/SphereComponent.h"
#include "THInteractPromptWidget.h"
#include "Player/THPlayerController.h"
#include "THItemBox.generated.h"



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
	void OpenBox(ATHPlayerController* PC);

	
	FName RandomItemGenerate(ATHPlayerController* PC);
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
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "plz")
	USkeletalMeshComponent* BoxMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimSequence* OpenAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimSequence* IdleAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimSequence* OpenIdleAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float OpenIdleDuration = 2.0f;

	FTimerHandle DestroyTimerHandle;

	void PlayOpenIdle();

	void OverlapDisable();



	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OpenBox();


	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOpenIdle();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundBase* EffectSound;
};
