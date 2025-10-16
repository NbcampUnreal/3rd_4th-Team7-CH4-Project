// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "THFootStepNotify.generated.h"

/**
 * 
 */
UCLASS()
class TREASUREHUNTER_API UTHFootStepNotify : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FootStep")
	FName FootSocket;
};
