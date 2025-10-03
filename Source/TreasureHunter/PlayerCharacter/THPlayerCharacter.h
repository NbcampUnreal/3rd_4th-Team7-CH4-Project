#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Item/THItemBox.h"
#include "Item/THBaseItem.h"
#include "PakourComponent/THCharacterMovementComponent.h"

#include "THPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UTHAttributeSet;
class UGameplayEffect;
class UMotionWarpingComponent;
class UNiagaraComponent;
struct UInputActionValue;
class UTHParkourComponent;
class UTHClimbComponent;
struct FInputActionValue;
struct FClimbTraceResult;

UCLASS()
class TREASUREHUNTER_API ATHPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

#pragma region Climb&Mantle
public:
	//ATHPlayerCharacter();

	ATHPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> ClimbAbilityClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> MantleAbilityClass;

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MantleAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ClimbAction;

	FORCEINLINE class UTHCharacterMovementComponent* GetTHMovement() const
	{
		return Cast<UTHCharacterMovementComponent>(GetCharacterMovement());
	}

	void OnClimbActionStarted(const FInputActionValue& Value);
	void OnParkourActionStarted(const FInputActionValue& Value);

public:
	void OnPlayerEnterClimbState();
	void OnPlayerExitClimbState();

#pragma endregion

	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE UTHParkourComponent* GetParkourComponent() const { return ParkourComponent; }
	FORCEINLINE UTHClimbComponent* GetClimbComponent() const { return ClimbComponent; }
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	FORCEINLINE UNiagaraComponent* GetFootStepComponent() const { return FootStepComponent; }
	
	float GetWalkSpeed() const;
	float GetSprintSpeed() const;

	void OnMantleStart();
	void OnMantleEnd();

	void EnterClimbState(const FVector& WallNormal);
	void LeaveClimbState();

	void CacheClimbStaminaEffects(const TSubclassOf<UGameplayEffect>& InDrainEffect, const TSubclassOf<UGameplayEffect>& InRegenEffect);
	void SwitchClimbStaminaEffect(bool bShouldRegen);
	void ClearClimbStaminaEffects();

protected:
	virtual void Jump() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_SetClimbingWallNormal(const FVector& InWallNormal);

	UFUNCTION(Server, Reliable)
	void Server_UpdateClimbingMovementState(bool bNewIsMoving);
	
public:
	void SetClimbingWallNormal(const FVector& InWallNormal);
	
	UFUNCTION()
	void OnRep_ClimbingWallNormal();

	UPROPERTY(ReplicatedUsing = OnRep_ClimbingWallNormal)
	FVector_NetQuantizeNormal ClimbingWallNormal;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Climb")
	bool bIsClimbing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "State|Climb")
	FVector2D ClimbMovementDirection = FVector2D::ZeroVector;
	
private:
	void InitializeAbilitySystem();
	void InitializeAbilitySystemCallbacks();
	
	void HandleMoveTriggered(const FInputActionValue& InValue);
	void HandleMoveCompleted(const FInputActionValue& InValue);
	void HandleLookInput(const FInputActionValue& InValue);
	void HandleClimbInput(const FInputActionValue& InValue);
	void HandleClimbInputReleased(const FInputActionValue& InValue);
	
	void HandleGroundMovement(const FVector2D& InMovementVector);
	void HandleClimbMovement(const FVector2D& InMovementVector);
	
	void UpdateClimbMovementState(const FVector2D& InMovementVector);
	void AdjustToClimbSurface();
	void ApplyClimbMovementInput(const FVector2D& InMovementVector);
	
	void RequestMantle(const FInputActionValue& InValue);
	void RequestClimb(const FInputActionValue& InValue);
	void RequestPush(const FInputActionValue& InValue);
	
	void OnSprintingStateChanged(const FGameplayTag Tag, int32 NewCount);
	void OnClimbingStateChanged(const FGameplayTag Tag, int32 NewCount);
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

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Climb")
	float MaxClimbSpeed = 150.f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTHParkourComponent> ParkourComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTHClimbComponent> ClimbComponent;

private:
	UPROPERTY()
	ATHItemBox* InteractableItemBox;
	
	UPROPERTY()
	ATHBaseItem* InteractableBaseItem;

	float DefaultMaxFlySpeed = 0.f;
	
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
	FActiveGameplayEffectHandle CurrentClimbStaminaEffectHandle;

	bool bIsClimbingAndMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraComponent> StunEffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraComponent> FootStepComponent;
	
	TSubclassOf<UGameplayEffect> ClimbStaminaDrainEffectClass;
	TSubclassOf<UGameplayEffect> ClimbStaminaRegenEffectClass;


};