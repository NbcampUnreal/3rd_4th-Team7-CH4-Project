#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "THJumpPad.generated.h"

class UStaticMeshComponent;
UCLASS()
class TREASUREHUNTER_API ATHJumpPad : public AActor
{
	GENERATED_BODY()
	
public:
	ATHJumpPad();



private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* JumpPadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad", meta = (AllowPrivateAccess = "true"))
    float LaunchStrength = 1000.f;

    UPROPERTY(Replicated)
    bool bIsActivated = true;

    UFUNCTION()
    void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ActivateJumpPad(ACharacter* CharacterToLaunch);

    UFUNCTION()
    void DestroyJumpPad();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
