#include "PlayerCharacter/THPlayerCharacter.h"
#include "Player/THPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "AttributeSet/THAttributeSet.h"
#include "Ability/THSprintAbility.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Item/THItemInventory.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Game/GameFlowTags.h"
#include "MotionWarpingComponent.h"
#include "GameplayEffect.h"

ATHPlayerCharacter::ATHPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// 컴포넌트 직접 속도 제어 말고 
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ATHPlayerCharacter::OnCapsuleHit);

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

void ATHPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocallyControlled() == true)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		checkf(IsValid(PC) == true, TEXT("EnhancedInputLocalPlayerSubsystem is invalid."))

		UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			PC->GetLocalPlayer());
		checkf(IsValid(EILPS) == true, TEXT("EnhancedInputLocalPlayerSubsystem is invalid."))

		EILPS->AddMappingContext(InputMappingContext, 0);
	}
}

void ATHPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		BindToAttributeChanges();

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->RegisterGameplayTagEvent(TAG_State_Debuff_Stun, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ATHPlayerCharacter::OnStunTagChanged);

			// 스프린트 상태 태그 콜백 등록
			ASC->RegisterGameplayTagEvent(TAG_State_Movement_Sprinting, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this, &ATHPlayerCharacter::OnSprintStateTagChanged);

			// 시작 시점 동기화(현재 태그 상태 반영)
			OnSprintStateTagChanged(TAG_State_Movement_Sprinting,
				ASC->HasMatchingGameplayTag(TAG_State_Movement_Sprinting) ? 1 : 0);
		}
	}

	UpdateMaxWalkSpeedFromAttributes();
}

void ATHPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
    
	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		BindToAttributeChanges();

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->RegisterGameplayTagEvent(TAG_State_Debuff_Stun, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ATHPlayerCharacter::OnStunTagChanged);

			// 스프린트 상태 태그 콜백 등록
			ASC->RegisterGameplayTagEvent(TAG_State_Movement_Sprinting, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this, &ATHPlayerCharacter::OnSprintStateTagChanged);

			// 시작 시점 동기화(현재 태그 상태 반영)
			OnSprintStateTagChanged(TAG_State_Movement_Sprinting,
				ASC->HasMatchingGameplayTag(TAG_State_Movement_Sprinting) ? 1 : 0);
		}
	}

	UpdateMaxWalkSpeedFromAttributes();
}

void ATHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveInput);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);
	EIC->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::ToggleCrouch);
	EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
	EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EIC->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::RequestSprint);
	EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::RequestSprint);
	EIC->BindAction(MantleAction, ETriggerEvent::Triggered, this, &ThisClass::RequestMantle);
	EIC->BindAction(PushAction, ETriggerEvent::Triggered, this, &ThisClass::RequestPush);
	EIC->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::OnInteract);
	EIC->BindAction(SlotUse1Action, ETriggerEvent::Triggered, this, &ThisClass::OnUseItemSlot1);
	EIC->BindAction(SlotUse2Action, ETriggerEvent::Triggered, this, &ThisClass::OnUseItemSlot2);
}

void ATHPlayerCharacter::HandleMoveInput(const FInputActionValue& InValue)
{
	if (ensure(IsValid(Controller)))
	{
		const FVector2D InMovementVector = InValue.Get<FVector2D>();
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);
		const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, InMovementVector.X);
		AddMovementInput(RightDirection, InMovementVector.Y);
	}
}

void ATHPlayerCharacter::HandleLookInput(const FInputActionValue& InValue)
{
	if (ensure(IsValid(Controller)))
	{
		const FVector2D InLookVector = InValue.Get<FVector2D>();

		AddControllerYawInput(InLookVector.X);
		AddControllerPitchInput(InLookVector.Y);
	}
}

float ATHPlayerCharacter::GetSprintSpeed() const
{
	if (const ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		if (const UTHAttributeSet* AS = PS->GetAttributeSet())
		{
			return AS->GetSprintSpeed();
		}
	}
	return 0.f;
}

float ATHPlayerCharacter::GetWalkSpeed() const
{
	if (const ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		if (const UTHAttributeSet* AS = PS->GetAttributeSet())
		{
			return AS->GetWalkSpeed();
		}
	}
	return 0.f;
}

void ATHPlayerCharacter::ToggleCrouch()
{
	if (bIsCrouched == false)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void ATHPlayerCharacter::RequestSprint(const FInputActionValue& InValue)
{
	const bool bIsPressed = InValue.Get<bool>();
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTagContainer SprintTagContainer(TAG_Ability_Sprint);
		
		if (bIsPressed == true)
		{
			ASC->TryActivateAbilitiesByTag(SprintTagContainer);
		}

		else
		{
			ASC->CancelAbilities(&SprintTagContainer);
		}
	}
}

void ATHPlayerCharacter::RequestPush(const FInputActionValue& InValue)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTag PushTag = FGameplayTag::RequestGameplayTag(FName("Ability.Push"));

		FGameplayTagContainer PushTagContainer(PushTag);
		
		ASC->TryActivateAbilitiesByTag(PushTagContainer);
	}
}

void ATHPlayerCharacter::RequestMantle(const FInputActionValue& InValue)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTagContainer MantleTagContainer(TAG_Ability_Mantle);
		ASC->TryActivateAbilitiesByTag(MantleTagContainer);
	}
}
void ATHPlayerCharacter::BindToAttributeChanges()
{
	if (auto* ASC = GetAbilitySystemComponent())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UTHAttributeSet::GetStaminaAttribute())
			.AddUObject(this, &ThisClass::OnStaminaChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UTHAttributeSet::GetWalkSpeedAttribute())
			.AddUObject(this, &ThisClass::OnWalkSpeedChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UTHAttributeSet::GetSprintSpeedAttribute())
			.AddUObject(this, &ThisClass::OnSprintSpeedChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UTHAttributeSet::GetJumpPowerAttribute())
			.AddUObject(this, &ThisClass::OnJumpPowerChanged);
	}
}

void ATHPlayerCharacter::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
}

UAbilitySystemComponent* ATHPlayerCharacter::GetAbilitySystemComponent() const
{
	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

//임시추가	

void ATHPlayerCharacter::SetInteractableActor(ATHItemBox* NewItemBox)
{
	InteractableItemBox = NewItemBox;
}
void ATHPlayerCharacter::SetInteractableBaseItem(ATHBaseItem* NewBaseItem)
{
	InteractableBaseItem = NewBaseItem;
}

void ATHPlayerCharacter::OnInteract()
{
	if (InteractableItemBox)
	{
		if (HasAuthority())
		{
			HandleBoxInteract();
		}
		else 
		{
			Server_HandleInteract(InteractableItemBox);
		}
	}
	else if (InteractableBaseItem)
	{
		if (HasAuthority())
		{
			// 서버에서 직접 처리
			HandleBaseItemInteract();
		}
		else
		{
			// 클라에서 서버에 요청
			Server_HandleBaseItemInteract(InteractableBaseItem);
		}
	}
}

bool ATHPlayerCharacter::Server_HandleInteract_Validate(ATHItemBox* InteractableBox)
{
	return true;
}

void ATHPlayerCharacter::Server_HandleInteract_Implementation(ATHItemBox* InteractableBox)
{
	if (InteractableBox)
	{
		InteractableBox->OpenBox();
	}
}

bool ATHPlayerCharacter::Server_HandleBaseItemInteract_Validate(ATHBaseItem* InteractableItem)
{
	return true;
}

void ATHPlayerCharacter::Server_HandleBaseItemInteract_Implementation(ATHBaseItem* InteractableItem)
{
	if (InteractableItem)
	{
		InteractableItem->ItemPickup(this);
	}
}

void ATHPlayerCharacter::HandleBoxInteract()
{
    if (InteractableItemBox)
    {
        InteractableItemBox->OpenBox();
        InteractableItemBox = nullptr;
    }
}

void ATHPlayerCharacter::HandleBaseItemInteract()
{
	if (InteractableBaseItem)
	{
		bool bPickedUp = InteractableBaseItem->ItemPickup(this);
		if (bPickedUp)
		{
			InteractableBaseItem = nullptr;
		}
	}
}

void ATHPlayerCharacter::OnUseItemSlot1()
{
	if (UTHItemInventory* Inventory = FindComponentByClass<UTHItemInventory>())
	{
		Inventory->Server_UseItem(1);
	}
}

void ATHPlayerCharacter::OnUseItemSlot2()
{
	if (UTHItemInventory* Inventory = FindComponentByClass<UTHItemInventory>())
	{
		Inventory->Server_UseItem(2);
	}
}


void ATHPlayerCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	UpdateMaxWalkSpeedFromAttributes();
}

void ATHPlayerCharacter::OnSprintSpeedChanged(const FOnAttributeChangeData& Data)
{
	UpdateMaxWalkSpeedFromAttributes();
}

void ATHPlayerCharacter::OnJumpPowerChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->JumpZVelocity = Data.NewValue;
	}
}

void ATHPlayerCharacter::Jump()
{
	float CurrentJumpPower = 0.f;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const UAttributeSet* BaseSet = ASC->GetAttributeSet(UTHAttributeSet::StaticClass());
		if (const UTHAttributeSet* AS = Cast<UTHAttributeSet>(BaseSet))
		{
			CurrentJumpPower = AS->GetJumpPower();
		}
	}

	GetCharacterMovement()->JumpZVelocity = CurrentJumpPower;

	Super::Jump();
}

void ATHPlayerCharacter::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ACharacter* OtherCharacter = Cast<ACharacter>(OtherActor))
	{
		if (GetCharacterMovement()->IsFalling())
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				FGameplayEventData EventData;
				EventData.Instigator = this;
				EventData.EventTag = TAG_Event_Hit_Falling;
				EventData.TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
				
				ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
			}
		}
		
	}
}

void ATHPlayerCharacter::OnStunTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		DisableInput(GetController<APlayerController>());
	}
	else
	{
		EnableInput(GetController<APlayerController>());
	}
}




void ATHPlayerCharacter::OnSprintStateTagChanged(const FGameplayTag /*Tag*/, int32 NewCount)
{
	bIsSprinting = (NewCount > 0);
	UpdateMaxWalkSpeedFromAttributes();
}

void ATHPlayerCharacter::UpdateMaxWalkSpeedFromAttributes()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (const UTHAttributeSet* AS = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass())))
		{
			const bool bSprintNow = ASC->HasMatchingGameplayTag(TAG_State_Movement_Sprinting);
			const float TargetSpeed = bSprintNow ? AS->GetSprintSpeed() : AS->GetWalkSpeed();

			if (UCharacterMovementComponent* CMC = GetCharacterMovement())
			{
				CMC->MaxWalkSpeed = TargetSpeed;
			}
		}
	}
}