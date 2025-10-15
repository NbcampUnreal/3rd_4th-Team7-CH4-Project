#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Item/THItemBox.h"
#include "Item/THBaseItem.h"
#include "Net/UnrealNetwork.h"
#include "THPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UTHAttributeSet;
class UGameplayEffect;
class UMotionWarpingComponent;
class UNiagaraComponent;
class UTHParkourComponent;
class UTHMovementComponent;

UCLASS()
class TREASUREHUNTER_API ATHPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATHPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyControllerChanged() override;

	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE UTHParkourComponent* GetParkourComponent() const { return ParkourComponent; }
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	FORCEINLINE UNiagaraComponent* GetFootStepComponent() const { return FootStepComponent; }
	FORCEINLINE UTHMovementComponent* GetTHMovementComponent() const { return THMovementComponent; }
	
	float GetWalkSpeed() const;
	float GetSprintSpeed() const;

	void OnMantleStart();
	void OnMantleEnd();

protected:
	virtual void BeginPlay() override;
	virtual void Jump() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void InitializeAbilitySystem();
	void InitializeAbilitySystemCallbacks();
	
	void HandleMoveInput(const FInputActionValue& InValue);
	void HandleLookInput(const FInputActionValue& InValue);
	
	void OnClimbActionStarted(const FInputActionValue& Value);
	void OnClimbHopActionStarted(const FInputActionValue& Value);
	void OnMoveInputCompleted(const FInputActionValue& InValue);

	UFUNCTION(Server, Reliable)
	void Server_ToggleClimbing();

	UFUNCTION(Server, Reliable)
	void Server_RequestHopping();
	
	void OnPlayerEnterClimbState();
	void OnPlayerExitClimbState();
	void AddInputMappingContext(const UInputMappingContext* ContextToAdd, int32 InPriority) const;
	void RemoveInputMappingContext(const UInputMappingContext* ContextRemove) const;
	
	void RequestMantle(const FInputActionValue& InValue);
	void RequestPush(const FInputActionValue& InValue);
	
	void OnSprintingStateChanged(const FGameplayTag Tag, int32 NewCount);
	void OnStaminaChanged(const FOnAttributeChangeData& Data);
	void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	void OnJumpPowerChanged(const FOnAttributeChangeData& Data);
	void OnSprintSpeedChanged(const FOnAttributeChangeData& Data);
	
	void OnMoveInputReleased(const FInputActionValue& InValue);
	void OnSprintPressed(const FInputActionValue&);
	void OnSprintReleased(const FInputActionValue&);
	
	void BindToAttributeChanges();
	void ToggleCrouch();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> ClimbMappingContext;

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
	TObjectPtr<UInputAction> MantleAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ClimbAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ClimbHopAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotUse1Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotUse2Action;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTHParkourComponent> ParkourComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTHMovementComponent> THMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsClimbing, Category = "Climb")
	bool bIsClimbing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Climb")
	FVector2D ClimbMovementDirection = FVector2D::ZeroVector;

private:
	UPROPERTY()
	ATHItemBox* InteractableItemBox;
	
	UPROPERTY()
	ATHBaseItem* InteractableBaseItem;

	UFUNCTION()
	void OnRep_IsClimbing();
	
public:	
	void SetInteractableActor(ATHItemBox* NewItemBox);
	void SetInteractableBaseItem(ATHBaseItem* NewBaseItem);
	
	void OnStunTagChanged(const FGameplayTag,int32 NewCount);
	
	UFUNCTION()
	void OnInteract();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HandleInteract(ATHItemBox* InteractableBox);
	void HandleBoxInteract();
	void HandleBaseItemInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HandleBaseItemInteract(ATHBaseItem* InteractableItem);

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnUseItemSlot1();
	
	UFUNCTION()
	void OnUseItemSlot2();
	
	bool bIsSprinting = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraComponent> StunEffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraComponent> FootStepComponent;
};