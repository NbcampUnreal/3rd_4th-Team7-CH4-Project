#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Item/THItemBox.h"
#include "Item/THBaseItem.h"

#include "THPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UTHAttributeSet;
class UGameplayEffect;
class UMotionWarpingComponent;
struct UInputActionValue;

UCLASS()
class TREASUREHUNTER_API ATHPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATHPlayerCharacter();

	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnStaminaChanged(const FOnAttributeChangeData& Data);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

	float GetWalkSpeed() const;

	float GetSprintSpeed() const;

private:
	void HandleMoveInput(const FInputActionValue& InValue);

	void HandleLookInput(const FInputActionValue& InValue);

	void RequestSprint(const FInputActionValue& InValue);

	void RequestMantle(const FInputActionValue& InValue);

	void RequestPush(const FInputActionValue& InValue);

	void BindToAttributeChanges();

	void ToggleCrouch();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PushAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotUse1Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotUse2Action;

private:
	UPROPERTY()
	ATHItemBox* InteractableItemBox;
	UPROPERTY()
	ATHBaseItem* InteractableBaseItem;

public:	
	void SetInteractableActor(ATHItemBox* NewItemBox);
	void SetInteractableBaseItem(ATHBaseItem* NewBaseItem);
	void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	void OnSprintSpeedChanged(const FOnAttributeChangeData& Data);

	bool bIsSprinting = false;
	
	UFUNCTION()
	void OnInteract();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HandleInteract(ATHItemBox* InteractableBox);
	void HandleBoxInteract();
	void HandleBaseItemInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HandleBaseItemInteract(ATHBaseItem* InteractableItem);

	UFUNCTION()
	void OnUseItemSlot1();
	UFUNCTION()
	void OnUseItemSlot2();

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MantleAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
};