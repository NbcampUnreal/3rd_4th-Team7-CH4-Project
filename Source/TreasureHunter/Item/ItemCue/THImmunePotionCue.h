#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "NiagaraComponent.h"
#include "GameplayTags.h"
#include "Game/GameFlowTags.h"
#include "THImmunePotionCue.generated.h"

UCLASS(meta = (GameplayCue = TAG_Cue_ImmunePotion))
class TREASUREHUNTER_API ATHImmunePotionCue : public AGameplayCueNotify_Actor
{
    GENERATED_BODY()

public:
    ATHImmunePotionCue();

protected:
    // 실행될 Niagara 이펙트
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UNiagaraSystem* ImmuneNiagara;

    // 실제 붙여지는 컴포넌트
    UPROPERTY()
    UNiagaraComponent* ActiveNiagaraComp;

public:    
    virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

    //virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

    virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

    UPROPERTY(EditAnywhere, Category = "Setting")
	FName SocketName = FName("Spine");

    UPROPERTY(EditAnywhere, Category = "Setting")
	FVector LocationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Setting")
	FRotator RotationOffset = FRotator::ZeroRotator;

};
