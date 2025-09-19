#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "THSpawnObject.generated.h"

UCLASS()
class TREASUREHUNTER_API ATHSpawnObject : public AActor
{
	GENERATED_BODY()
	
public:
	ATHSpawnObject();
		
	UPROPERTY(ReplicatedUsing = OnRep_PlacerActor)
	TObjectPtr<AActor> PlacerActor;

	UFUNCTION(BlueprintCallable, Category = "Placer")
	void SetPlacerActor(AActor* NewPlacer);

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_PlacerActor();

	UFUNCTION()
	virtual void OnPlacerActorReplicated();
};
