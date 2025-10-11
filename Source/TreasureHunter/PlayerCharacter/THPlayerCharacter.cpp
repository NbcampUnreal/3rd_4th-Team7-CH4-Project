#include "PlayerCharacter/THPlayerCharacter.h"
#include "Player/THPlayerState.h"
#include "Game/GameFlowTags.h"
#include "AttributeSet/THAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Item/THItemInventory.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "MotionWarpingComponent.h"
#include "NiagaraComponent.h"
#include "ParkourComponent/THParkourComponent.h"
#include "ParkourComponent/THMovementComponent.h"

ATHPlayerCharacter::ATHPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTHMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	THMovementComponent = Cast<UTHMovementComponent>(GetCharacterMovement());
	
	if (THMovementComponent)
	{
		THMovementComponent->bOrientRotationToMovement = true;
		THMovementComponent->RotationRate = FRotator(0.f, 540.f, 0.f);
		THMovementComponent->bUseControllerDesiredRotation = false;
		THMovementComponent->MaxWalkSpeed = 200.f;
		THMovementComponent->NavAgentProps.bCanCrouch = true;
	}
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ATHPlayerCharacter::OnCapsuleHit);

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	StunEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StunEffectComponent"));
	StunEffectComponent->SetupAttachment(GetMesh(), TEXT("StunEffectSocket"));
	StunEffectComponent->bAutoActivate = false;

	FootStepComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FootStepComponent"));
	FootStepComponent->SetupAttachment(GetMesh(), TEXT("FootStep"));
	FootStepComponent->bAutoActivate = false;

	ParkourComponent = CreateDefaultSubobject<UTHParkourComponent>(TEXT("ParkourComponent"));
}

void ATHPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ATHPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void ATHPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

void ATHPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	AddInputMappingContext(DefaultMappingContext, 0);

	if (THMovementComponent)
	{
		THMovementComponent->OnEnterClimbStateDelegate.BindUObject(this, &ThisClass::OnPlayerEnterClimbState);
		THMovementComponent->OnExitClimbStateDelegate.BindUObject(this, &ThisClass::OnPlayerExitClimbState);
	}
}

void ATHPlayerCharacter::InitializeAbilitySystem()
{
	ATHPlayerState* PS = GetPlayerState<ATHPlayerState>();
	if (!PS) return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC) return;
	
	ASC->InitAbilityActorInfo(PS, this);
	
	InitializeAbilitySystemCallbacks();
}

void ATHPlayerCharacter::InitializeAbilitySystemCallbacks()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ensure(ASC)) return;

	BindToAttributeChanges();

	ASC->RegisterGameplayTagEvent(TAG_State_Debuff_Stun, EGameplayTagEventType::NewOrRemoved)
	   .AddUObject(this, &ATHPlayerCharacter::OnStunTagChanged);
	
	ASC->RegisterGameplayTagEvent(TAG_State_Movement_Sprinting, EGameplayTagEventType::NewOrRemoved)
	   .AddUObject(this, &ATHPlayerCharacter::OnSprintingStateChanged);
}

void ATHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveInput);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::OnMoveInputCompleted);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);
		EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EIC->BindAction(MantleAction, ETriggerEvent::Triggered, this, &ThisClass::RequestMantle);
		EIC->BindAction(PushAction, ETriggerEvent::Triggered, this, &ThisClass::RequestPush);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::OnMoveInputReleased);
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::OnSprintPressed);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::OnSprintReleased);
		EIC->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::OnInteract);
		EIC->BindAction(SlotUse1Action, ETriggerEvent::Triggered, this, &ThisClass::OnUseItemSlot1);
		EIC->BindAction(SlotUse2Action, ETriggerEvent::Triggered, this, &ThisClass::OnUseItemSlot2);
		EIC->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::ToggleCrouch);
		EIC->BindAction(ClimbAction, ETriggerEvent::Started, this, &ThisClass::OnClimbActionStarted);
		EIC->BindAction(ClimbHopAction, ETriggerEvent::Started, this, &ThisClass::OnClimbHopActionStarted);
	}
}

void ATHPlayerCharacter::HandleMoveInput(const FInputActionValue& InValue)
{
	if (THMovementComponent && THMovementComponent->IsClimbing())
	{
		const FVector2D MovementVector = InValue.Get<FVector2D>();
		this->ClimbMovementDirection = MovementVector;
		const FVector UpDirection = GetActorUpVector();
		const FVector RightDirection = GetActorRightVector();
		
		AddMovementInput(UpDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}

	else
	{
		if (IsValid(Controller))
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
}

void ATHPlayerCharacter::OnMoveInputReleased(const FInputActionValue& InValue)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
	   if (ASC->HasMatchingGameplayTag(TAG_State_Movement_Sprinting))
	   {
		 FGameplayEventData Payload;
		 UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Movement_Stopped, Payload);
	   }
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

void ATHPlayerCharacter::OnClimbActionStarted(const FInputActionValue& Value)
{
	if (!THMovementComponent) return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ASC->HasMatchingGameplayTag(TAG_Status_State_Mantling)) return;
		if (ASC->HasMatchingGameplayTag(TAG_State_Debuff_Stun)) return;
		if (ASC->HasMatchingGameplayTag(TAG_Status_Stamina_Empty)) return;
	}
	if (bIsCrouched) return;
	
	if (HasAuthority())
	{
		THMovementComponent->ToggleClimbing(!THMovementComponent->IsClimbing());
	}
	else
	{
		Server_ToggleClimbing();
	}
}

void ATHPlayerCharacter::OnMoveInputCompleted(const FInputActionValue& InValue)
{
	if (THMovementComponent && THMovementComponent->IsClimbing())
	{
		ClimbMovementDirection = FVector2D::ZeroVector;
	}
}

void ATHPlayerCharacter::OnClimbHopActionStarted(const FInputActionValue& Value)
{
	if (!THMovementComponent) return;
	
	if (HasAuthority())
	{
		THMovementComponent->RequestHopping();
	}
	else
	{
		Server_RequestHopping();
	}
}

void ATHPlayerCharacter::Server_ToggleClimbing_Implementation()
{
	if (THMovementComponent)
	{
		THMovementComponent->ToggleClimbing(!THMovementComponent->IsClimbing());
	}
}

void ATHPlayerCharacter::Server_RequestHopping_Implementation()
{
	if (THMovementComponent)
	{
		THMovementComponent->RequestHopping();
	}
}

void ATHPlayerCharacter::OnPlayerEnterClimbState()
{
}

void ATHPlayerCharacter::OnPlayerExitClimbState()
{
}

void ATHPlayerCharacter::AddInputMappingContext(const UInputMappingContext* ContextToAdd, const int32 InPriority) const
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ContextToAdd, InPriority);
		}
	}
}

void ATHPlayerCharacter::RemoveInputMappingContext(const UInputMappingContext* ContextRemove) const
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(ContextRemove);
		}
	}
}

void ATHPlayerCharacter::HandleBoxInteract()
{
	if (InteractableItemBox)
	{
		ATHPlayerController* PC = Cast<ATHPlayerController>(GetController());
		InteractableItemBox->OpenBox(PC);
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

UAbilitySystemComponent* ATHPlayerCharacter::GetAbilitySystemComponent() const
{
	if (ATHPlayerState* PS = GetPlayerState<ATHPlayerState>())
	{
	   return PS->GetAbilitySystemComponent();
	}
	return nullptr;
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

void ATHPlayerCharacter::OnSprintPressed(const FInputActionValue&)
{
	if (THMovementComponent && THMovementComponent->IsClimbing()) return;
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
	   FGameplayTagContainer SprintTags(TAG_Ability_Sprint);
	   ASC->TryActivateAbilitiesByTag(SprintTags);
	}
}

void ATHPlayerCharacter::OnSprintReleased(const FInputActionValue&)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
	   FGameplayTagContainer SprintTags(TAG_Ability_Sprint);
	   ASC->CancelAbilities(&SprintTags);
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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC)
	{
	   const UTHAttributeSet* AS = Cast<UTHAttributeSet>(ASC->GetAttributeSet(UTHAttributeSet::StaticClass()));
	   if (AS)
	   {
		 ASC->GetGameplayAttributeValueChangeDelegate(AS->GetStaminaAttribute()).AddUObject(this, &ATHPlayerCharacter::OnStaminaChanged);
		 ASC->GetGameplayAttributeValueChangeDelegate(AS->GetWalkSpeedAttribute()).AddUObject(this, &ATHPlayerCharacter::OnWalkSpeedChanged);
		 ASC->GetGameplayAttributeValueChangeDelegate(AS->GetSprintSpeedAttribute()).AddUObject(this, &ATHPlayerCharacter::OnSprintSpeedChanged);
		 ASC->GetGameplayAttributeValueChangeDelegate(AS->GetJumpPowerAttribute()).AddUObject(this, &ATHPlayerCharacter::OnJumpPowerChanged);
	   }
	}
}

void ATHPlayerCharacter::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
}

void ATHPlayerCharacter::OnSprintingStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	const UTHAttributeSet* AS = Cast<UTHAttributeSet>(GetAbilitySystemComponent()->GetAttributeSet(UTHAttributeSet::StaticClass()));
	if (!AS) return;
	
	bIsSprinting = NewCount > 0;
	if (THMovementComponent)
	{
		THMovementComponent->MaxWalkSpeed = bIsSprinting ? AS->GetSprintSpeed() : AS->GetWalkSpeed();
	}
}

void ATHPlayerCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (!bIsSprinting && THMovementComponent)
	{
	   THMovementComponent->MaxWalkSpeed = Data.NewValue;
	}
}

void ATHPlayerCharacter::OnSprintSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (bIsSprinting && THMovementComponent)
	{
	   THMovementComponent->MaxWalkSpeed = Data.NewValue;
	}
}

void ATHPlayerCharacter::OnJumpPowerChanged(const FOnAttributeChangeData& Data)
{
	if (THMovementComponent)
	{
	   THMovementComponent->JumpZVelocity = Data.NewValue;
	}
}

void ATHPlayerCharacter::OnStunTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		DisableInput(GetController<APlayerController>());
		if (StunEffectComponent)
		{
			StunEffectComponent->Activate(true);
		}
	}
	else
	{
		EnableInput(GetController<APlayerController>());
		if (StunEffectComponent)
		{
			StunEffectComponent->Deactivate();
		}
	}
}

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
			HandleBaseItemInteract();
		}
		else
		{
			Server_HandleBaseItemInteract(InteractableBaseItem);
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

	if (THMovementComponent)
	{
		THMovementComponent->JumpZVelocity = CurrentJumpPower;
	}

	Super::Jump();
}

void ATHPlayerCharacter::Server_HandleInteract_Implementation(ATHItemBox* InteractableBox)
{
	if (InteractableBox)
	{	
		ATHPlayerController* PC = Cast<ATHPlayerController>(GetController());
		InteractableBox->OpenBox(PC);
	}
}

void ATHPlayerCharacter::Server_HandleBaseItemInteract_Implementation(ATHBaseItem* InteractableItem)
{
	if (InteractableItem)
	{
		InteractableItem->ItemPickup(this);
	}
}

bool ATHPlayerCharacter::Server_HandleInteract_Validate(ATHItemBox* InteractableBox)
{
	return true;
}

bool ATHPlayerCharacter::Server_HandleBaseItemInteract_Validate(ATHBaseItem* InteractableItem)
{
	return true;
}

void ATHPlayerCharacter::OnMantleStart()
{
	if (GetCapsuleComponent())
	{
	   GetCapsuleComponent()->SetCollisionProfileName(FName("Mantling"));
	}
	if (GetSpringArm())
	{
	   GetSpringArm()->bDoCollisionTest = false;
	}
	if (GetCharacterMovement())
	{
	   GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
}

void ATHPlayerCharacter::OnMantleEnd()
{
	if (GetSpringArm())
	{
	   GetSpringArm()->bDoCollisionTest = true;
	}
	if (GetCapsuleComponent())
	{
	   GetCapsuleComponent()->SetCollisionProfileName(FName("Pawn"));
	}
	if (GetCharacterMovement())
	{
	   GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}