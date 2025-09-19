#pragma once

#include "CoreMinimal.h"
#include "Item/Spawn/THSpawnObject.h"
#include "THSlowTrap.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UGameplayEffect;

UCLASS()
class TREASUREHUNTER_API ATHSlowTrap : public ATHSpawnObject
{
	GENERATED_BODY()
	
public:
	ATHSlowTrap();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TrapMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(Replicated)
	bool bIsActive;

	UFUNCTION()
	void ApplySlowEffect(AActor* TargetActor);

	void ActivateTrap();

	FTimerHandle ActivationTimerHandle;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
	virtual void OnPlacerActorReplicated() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trap|Ability")
	TSubclassOf<class UGameplayAbility> SlowAbilityClass;

};
