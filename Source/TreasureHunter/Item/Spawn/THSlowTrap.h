#pragma once

#include "THSpawnObject.h"
#include "GameplayTagContainer.h"
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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnPlacerActorReplicated() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TrapMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* OverlapSphere;

    UPROPERTY(Replicated)
    bool bIsActive = false;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // 서버에서 적용되는 Slow GE 
    UPROPERTY(EditDefaultsOnly, Category = "Trap|Effect")
    TSubclassOf<UGameplayEffect> SlowGEClass;

    UPROPERTY(EditAnywhere, Category = "Trap|Effect") // 음수로 세팅 -> 아래 나오는 값들은 생성자에서 초기화 하는 게 더 좋음
    float WalkDelta = -150.f;

    UPROPERTY(EditAnywhere, Category = "Trap|Effect") // 음수로 세팅
    float SprintDelta = -500.f;

    UPROPERTY(EditAnywhere, Category = "Trap|Effect")
    float DurationSec = 5.f;

    // 면역 태그
    UPROPERTY(EditDefaultsOnly, Category = "Trap|Tags")
    FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag("Item.ImmunePotion.Active");

    UPROPERTY(EditDefaultsOnly, Category = "Trap|Tags")
    FGameplayTag GrantedActiveTag = FGameplayTag::RequestGameplayTag("Item.SpeedSlow.Active");

    // 활성 지연(스폰 후 N초 뒤부터 작동)
    UPROPERTY(EditAnywhere, Category = "Trap")
    float ActivateDelaySec = 0.2f;

private:
    void ActivateTrap();
    void ApplySlowTo(AActor* TargetActor);

public:
    UFUNCTION()
    void InitFromItemRow(TSubclassOf<UGameplayEffect> InGE, float InWalkDelta, float InSprintDelta, float InDurationSec);
};
