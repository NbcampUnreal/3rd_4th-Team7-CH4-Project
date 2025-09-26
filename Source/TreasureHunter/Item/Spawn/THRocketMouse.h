#pragma once

#include "CoreMinimal.h"
#include "Item/Spawn/THSpawnObject.h"
#include "THRocketMouse.generated.h"

class ATHRocketMouseBomb;

UCLASS()
class TREASUREHUNTER_API ATHRocketMouse : public ATHSpawnObject
{
    GENERATED_BODY()

public:
    ATHRocketMouse();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void OnPlacerActorReady();

    UPROPERTY(Replicated)
    FVector InitialVelocity;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    TSubclassOf<class ATHRocketMouseBomb> RocketMouseBombClass;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float InitialSpeed = 1500.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
	float UpAngleDeg = 30.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SpawnDelay = 3.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
	FVector BombOffset = FVector(3000.f, 0.f, 3000.f);


//public:
//    UFUNCTION(NetMulticast, Reliable)
//    void Multicast_ItemUseEffect();
//
//
//    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
//    USoundBase* EffectSound;
};
