#include "Item/ItemCue/THImmunePotionCue.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"

ATHImmunePotionCue::ATHImmunePotionCue()
{
    bAutoDestroyOnRemove = true;
}

bool ATHImmunePotionCue::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
    if (!ImmuneNiagara || !MyTarget) return false;

    USkeletalMeshComponent* Mesh = MyTarget->FindComponentByClass<USkeletalMeshComponent>();
    if (Mesh)
    {
        ActiveNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            ImmuneNiagara,
            Mesh,
            SocketName, 
            LocationOffset,
            RotationOffset,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    else
    {
        ActiveNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            MyTarget->GetWorld(),
            ImmuneNiagara,
            MyTarget->GetActorLocation()
        );
    }

    return true;
}

//bool ATHImmunePotionCue::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
//{
//    return true;
//}

bool ATHImmunePotionCue::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
    if (ActiveNiagaraComp)
    {
        ActiveNiagaraComp->Deactivate();
        ActiveNiagaraComp = nullptr;
    }
    return true;
}
