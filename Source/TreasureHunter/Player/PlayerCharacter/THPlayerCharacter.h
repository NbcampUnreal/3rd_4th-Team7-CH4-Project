#pragma once

#pragma region Includes
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"

#include "THPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UTHAttributeSet;
class UGameplayEffect;
class UMotionWarpingComponent;
class ATHItemBox;
class ATHBaseItem;
struct FInputActionValue;
#pragma endregion

/*
함수 정의는 외부에서 쓰는 거 아니면 최대한 private으로
그리고 가독성을 위해서 카테고리 별로 나누고, 선언 순서대로 정의하는 습관 
정의하지 않은 함수들은 지우는 게 좋음 
*/

UCLASS() 
class TREASUREHUNTER_API ATHPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATHPlayerCharacter();

	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

#pragma region Movement
public:
	float GetWalkSpeed() const;
	float GetSprintSpeed() const;

private:
	void HandleMoveInput(const FInputActionValue& InValue);
	void OnMoveInputReleased(const FInputActionValue& InValue);
	void HandleLookInput(const FInputActionValue& InValue);

	void OnSprintPressed(const FInputActionValue&);
	void OnSprintReleased(const FInputActionValue&);

	void RequestMantle(const FInputActionValue& InValue);
	void RequestPush(const FInputActionValue& InValue);

	void BindToAttributeChanges();
	void ToggleCrouch();

	void UpdateMaxWalkSpeedFromAttributes(); // 걷기/스프린트 값 중 현재 상태에 맞는 값으로 CMC 갱신

	void OnStunTagChanged(const FGameplayTag, int32 NewCount);
	void OnSprintStateTagChanged(const FGameplayTag Tag, int32 NewCount); // 스프린트 상태 태그 변화 콜백

public:
	void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	void OnSprintSpeedChanged(const FOnAttributeChangeData& Data);
	void OnJumpPowerChanged(const FOnAttributeChangeData& Data);

protected:
	virtual void Jump() override;
private:
	bool bIsSprinting = false;
#pragma endregion

#pragma region InputActions
public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MantleAction;

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
#pragma endregion

#pragma region ItemInteraction
private:
	UPROPERTY()
	ATHItemBox* InteractableItemBox;
	UPROPERTY()
	ATHBaseItem* InteractableBaseItem;

public:	
	void SetInteractableActor(ATHItemBox* NewItemBox);
	void SetInteractableBaseItem(ATHBaseItem* NewBaseItem);

public:
	UFUNCTION()
	void OnInteract();
	UFUNCTION(Server, Reliable) // [FIX] UE5.4 이후부터는 이거 안씀 
	void Server_HandleInteract(ATHItemBox* InteractableBox);
	UFUNCTION(Server, Reliable)
	void Server_HandleBaseItemInteract(ATHBaseItem* InteractableItem);

	UFUNCTION()
	void OnUseItemSlot1();
	UFUNCTION()
	void OnUseItemSlot2();

private:
	void HandleBoxInteract();
	void HandleBaseItemInteract();
#pragma endregion

#pragma region Components
public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
#pragma endregion
};