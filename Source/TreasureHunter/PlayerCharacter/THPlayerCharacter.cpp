#include "PlayerCharacter/THPlayerCharacter.h"
#include "Player/THPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "AttributeSet/THAttributeSet.h"
#include "Ability/THSprintAbility.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameplayEffect.h"
#include "Components/CapsuleComponent.h"
#include "Item/THItemInventory.h"

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
	
	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ATHPlayerCharacter::OnCapsuleHit);

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
	}
}

void ATHPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
    
	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		BindToAttributeChanges();
	}
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
		FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(FName("Ability.Sprint"));

		FGameplayTagContainer SprintTagContainer(SprintTag);
		
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

void ATHPlayerCharacter::BindToAttributeChanges()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC)
	{
		const UTHAttributeSet* AS = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass()));
		if (AS)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(AS->GetStaminaAttribute()).AddUObject(this, &ATHPlayerCharacter::OnStaminaChanged);
		}

		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetWalkSpeedAttribute()).AddUObject(this, &ATHPlayerCharacter::OnWalkSpeedChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetSprintSpeedAttribute()).AddUObject(this, &ATHPlayerCharacter::OnSprintSpeedChanged);
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
		InteractableItemBox->OpenBox();
		
		InteractableItemBox = nullptr;
	}

	if (InteractableBaseItem)
	{
		if (InteractableBaseItem->bIsPickedUp)
		{
			InteractableBaseItem->ItemPickup(this);

			InteractableBaseItem = nullptr;
		}
	}
}

void ATHPlayerCharacter::OnUseItemSlot1()
{
	if (UTHItemInventory* Inventory = FindComponentByClass<UTHItemInventory>())
	{
		Inventory->UseItem(1);
	}
}

void ATHPlayerCharacter::OnUseItemSlot2()
{
	if (UTHItemInventory* Inventory = FindComponentByClass<UTHItemInventory>())
	{
		Inventory->UseItem(2);
	}
}

void ATHPlayerCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (!bIsSprinting) // 스프린트 중이 아닐 때만 워킹 속도를 업데이트
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void ATHPlayerCharacter::OnSprintSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (bIsSprinting) // 스프린트 중일 때만 업데이트
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void ATHPlayerCharacter::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CapsuleHit")));
}


