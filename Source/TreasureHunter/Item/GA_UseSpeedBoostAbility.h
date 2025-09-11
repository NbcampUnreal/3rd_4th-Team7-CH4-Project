#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_UseSpeedBoostAbility.generated.h"

UCLASS()
class TREASUREHUNTER_API UGA_UseSpeedBoostAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// 이펙트 클래스에 접근하기 위한 변수 (에디터에서 지정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<class UGameplayEffect> SpeedBoostEffectClass;

protected:
	// 어빌리티가 발동되었을 때 실행되는 핵심 함수
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
