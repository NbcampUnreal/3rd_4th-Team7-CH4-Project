#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Abilities/GameplayAbility.h"
#include "THRocketMouseBomb.generated.h"

UCLASS()
class TREASUREHUNTER_API ATHRocketMouseBomb : public AActor
{
	GENERATED_BODY()
	
public:
    ATHRocketMouseBomb();
    
    void SetTarget(AActor* NewTarget);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    class USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(ReplicatedUsing = OnRep_Velocity)
    FVector ReplicatedVelocity;

    UFUNCTION()
    void OnRep_Velocity();

    UPROPERTY(Replicated)
    AActor* TargetActor;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Explode();

    UPROPERTY(EditAnywhere, Category = "Effect")
    class UNiagaraSystem* ExplosionNiagara;

    UPROPERTY(EditAnywhere, Category = "Effect")
    float FireSpeed = 100.f;

    void ApplyAbilityToTarget();

    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    TSubclassOf<UGameplayAbility> ExplosionAbility;


public:

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ItemUseEffect();

    void ExplodeSound();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
    USoundBase* WingEffectSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
    USoundBase* BombEffectSound;
};
