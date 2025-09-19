#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "THAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class TREASUREHUNTER_API UTHAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UTHAttributeSet();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    
    UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_Stamina)
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UTHAttributeSet, Stamina)

    UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_MaxStamina)
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UTHAttributeSet, MaxStamina)

    UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_WalkSpeed)
    FGameplayAttributeData WalkSpeed;
    ATTRIBUTE_ACCESSORS(UTHAttributeSet, WalkSpeed)

    UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_SprintSpeed)
    FGameplayAttributeData SprintSpeed;
    ATTRIBUTE_ACCESSORS(UTHAttributeSet, SprintSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_JumpPower)
	FGameplayAttributeData JumpPower;
	ATTRIBUTE_ACCESSORS(UTHAttributeSet, JumpPower)

	UPROPERTY(BlueprintReadOnly, Category = "Widget", ReplicatedUsing = OnRep_OverlayWidget)
	FGameplayAttributeData OverlayWidget;
	ATTRIBUTE_ACCESSORS(UTHAttributeSet, OverlayWidget)

protected:
    UFUNCTION()
    virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

    UFUNCTION()
    virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

    UFUNCTION()
    virtual void OnRep_WalkSpeed(const FGameplayAttributeData& OldWalkSpeed);

    UFUNCTION()
    virtual void OnRep_SprintSpeed(const FGameplayAttributeData& OldSprintSpeed);

	UFUNCTION()
	virtual void OnRep_JumpPower(const FGameplayAttributeData& OldJumpPower);

	UFUNCTION()
	virtual void OnRep_OverlayWidget(const FGameplayAttributeData& OldOverlayWidget);
};