#include "PlayerCharacter/THPlayerCharacter.h"
#include "Player/THPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Game/GameFlowTags.h"
#include "AttributeSet/THAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Item/THItemInventory.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ParkourComponent/THClimbComponent.h"
#include "ParkourComponent/THParkourComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "MotionWarpingComponent.h"
#include "GameplayEffect.h"
#include "NiagaraComponent.h"

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

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	StunEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StunEffectComponent"));
	StunEffectComponent->SetupAttachment(GetMesh(), TEXT("StunEffectSocket"));
	StunEffectComponent->bAutoActivate = false;

	FootStepComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FootStepComponent"));
	FootStepComponent->SetupAttachment(GetMesh(), TEXT("FootStep"));
	FootStepComponent->bAutoActivate = false;

	ParkourComponent = CreateDefaultSubobject<UTHParkourComponent>(TEXT("ParkourComponent"));
	ClimbComponent = CreateDefaultSubobject<UTHClimbComponent>(TEXT("ClimbComponent"));
	
	if (GetCharacterMovement())
	{
	   DefaultMaxFlySpeed = GetCharacterMovement()->MaxFlySpeed;
	}
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

void ATHPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATHPlayerCharacter, ClimbingWallNormal);
	DOREPLIFETIME(ATHPlayerCharacter, ClimbMovementDirection);
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
	
	ASC->RegisterGameplayTagEvent(TAG_State_Movement_Climbing, EGameplayTagEventType::NewOrRemoved)
	   .AddUObject(this, &ATHPlayerCharacter::OnClimbingStateChanged);
}


void ATHPlayerCharacter::EnterClimbState(const FVector& InWallNormal)
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC) return;

	CMC->SetMovementMode(MOVE_Flying);
	CMC->bOrientRotationToMovement = false;
	CMC->Velocity = FVector::ZeroVector;
	CMC->BrakingDecelerationFlying = 1000.f;
	CMC->MaxFlySpeed = MaxClimbSpeed;
	
	CMC->SetPlaneConstraintEnabled(true);
	CMC->SetPlaneConstraintNormal(InWallNormal);

	SetClimbingWallNormal(InWallNormal);

	bIsClimbing = true;
}

void ATHPlayerCharacter::LeaveClimbState()
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC) return;
	
	const FVector DetachOffset = ClimbingWallNormal * 15.f;
	AddActorWorldOffset(DetachOffset);

	CMC->Velocity = FVector::ZeroVector;
	CMC->SetMovementMode(MOVE_Falling);
	CMC->bOrientRotationToMovement = true;
	CMC->bUseControllerDesiredRotation = false;
	CMC->MaxFlySpeed = DefaultMaxFlySpeed;
	CMC->BrakingDecelerationFlying = 0.f;
	CMC->SetPlaneConstraintEnabled(false);

	bIsClimbing = false;
	ClimbMovementDirection = FVector2D::ZeroVector;
}

void ATHPlayerCharacter::SetClimbingWallNormal(const FVector& InWallNormal)
{
	if (HasAuthority())
	{
	   ClimbingWallNormal = InWallNormal;
	   OnRep_ClimbingWallNormal();
	}
	else
	{
	   Server_SetClimbingWallNormal(InWallNormal);
	}
}

void ATHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveTriggered);
	EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::HandleMoveCompleted);
	EIC->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ThisClass::HandleMoveCompleted);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);
	EIC->BindAction(ClimbAction, ETriggerEvent::Started, this, &ThisClass::HandleClimbInput);
	EIC->BindAction(ClimbAction, ETriggerEvent::Completed, this, &ThisClass::HandleClimbInputReleased);
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
}

void ATHPlayerCharacter::HandleMoveTriggered(const FInputActionValue& InValue)
{
	const FVector2D InMovementVector = InValue.Get<FVector2D>();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (ASC && ASC->HasMatchingGameplayTag(TAG_State_Movement_Climbing))
	{
	   HandleClimbMovement(InMovementVector);
	}
	else
	{
	   HandleGroundMovement(InMovementVector);
	}
}

void ATHPlayerCharacter::HandleGroundMovement(const FVector2D& InMovementVector)
{
	if (IsValid(Controller))
	{
	   const FRotator ControlRotation = Controller->GetControlRotation();
	   const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	   const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
	   const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

	   AddMovementInput(ForwardDirection, InMovementVector.X);
	   AddMovementInput(RightDirection, InMovementVector.Y);
	}
}

void ATHPlayerCharacter::HandleClimbMovement(const FVector2D& InMovementVector)
{
	if (!ClimbComponent) return;

	UpdateClimbMovementState(InMovementVector);
	AdjustToClimbSurface();
	ApplyClimbMovementInput(InMovementVector);
}

void ATHPlayerCharacter::UpdateClimbMovementState(const FVector2D& InMovementVector)
{
	ClimbMovementDirection = InMovementVector;

	const bool bIsNowMoving = !InMovementVector.IsNearlyZero();
	
	if (bIsNowMoving != bIsClimbingAndMoving)
	{
		bIsClimbingAndMoving = bIsNowMoving;
		Server_UpdateClimbingMovementState(bIsClimbingAndMoving);
	}
}

void ATHPlayerCharacter::AdjustToClimbSurface()
{
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	FHitResult FrontHit;
	const FVector TraceStart = GetActorLocation() - GetActorForwardVector() * 10.f;
	const FVector TraceEnd = TraceStart + GetActorForwardVector() * (CapsuleRadius + 30.f);

	bool bWallFoundInFront = UKismetSystemLibrary::SphereTraceSingle(
	   GetWorld(), TraceStart, TraceEnd, 5.f,
	   UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
	   false, { this }, EDrawDebugTrace::None, FrontHit, true);

	if (bWallFoundInFront)
	{
	   const bool bIsValidWall = FMath::Abs(FrontHit.ImpactNormal.Z) < 0.1f;
	   const float NormalDotProduct = FVector::DotProduct(ClimbingWallNormal, FrontHit.ImpactNormal);
	   const bool bIsSimilarDirection = NormalDotProduct > 0.9f;

	   if (bIsValidWall && (ClimbingWallNormal.IsZero() || bIsSimilarDirection))
	   {
		  GetCharacterMovement()->SetPlaneConstraintNormal(FrontHit.ImpactNormal);
		  SetClimbingWallNormal(FrontHit.ImpactNormal);
		  const FVector TargetLocation = FrontHit.ImpactPoint + FrontHit.ImpactNormal * CapsuleRadius;
		  SetActorLocation(FVector(TargetLocation.X, TargetLocation.Y, GetActorLocation().Z));
	   }
	}
}

void ATHPlayerCharacter::ApplyClimbMovementInput(const FVector2D& InMovementVector)
{
	const FVector UpDownDirection = FVector::CrossProduct(GetActorRightVector(), ClimbingWallNormal);
	const FVector LeftRightDirection = GetActorRightVector();
	
	if (!FMath::IsNearlyZero(InMovementVector.Y))
	{
	   const FVector DesiredSidewaysDir = LeftRightDirection * FMath::Sign(InMovementVector.Y);
	   FClimbTraceResult SidewaysHitResult;

	   if (ClimbComponent && ClimbComponent->FindClimbableSurface(DesiredSidewaysDir, SidewaysHitResult))
	   {
		  SetClimbingWallNormal(SidewaysHitResult.WallNormal);
		  AddMovementInput(LeftRightDirection, InMovementVector.Y);
	   }
	}
	
	if (!FMath::IsNearlyZero(InMovementVector.X))
	{
	   if (InMovementVector.X > 0.f)
	   {
		  const FVector Start = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		  const FVector End = Start + GetActorForwardVector() * 100.f;
		  FHitResult UpwardHit;

		  bool bWallAbove = UKismetSystemLibrary::SphereTraceSingle(
			 GetWorld(), Start, End, 5.f,
			 UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
			 false, { this }, EDrawDebugTrace::None, UpwardHit, true);
	   	
		  if (bWallAbove)
		  {
			 AddMovementInput(UpDownDirection, InMovementVector.X);
		  }
	   }
	   else
	   {
		  AddMovementInput(UpDownDirection, InMovementVector.X);
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

void ATHPlayerCharacter::HandleClimbInput(const FInputActionValue& InValue)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	if (ASC->HasMatchingGameplayTag(TAG_State_Movement_Climbing))
	{
	   FGameplayTagContainer TagsToCancel(TAG_Ability_Climb);
	   ASC->CancelAbilities(&TagsToCancel);
	}
	else
	{
	   FGameplayTagContainer TagsToActivate(TAG_Ability_Climb);
	   ASC->TryActivateAbilitiesByTag(TagsToActivate);
	}
}

void ATHPlayerCharacter::HandleClimbInputReleased(const FInputActionValue& InValue)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
	   if (ASC->HasMatchingGameplayTag(TAG_State_Movement_Climbing))
	   {
		 FGameplayTagContainer TagsToCancel(TAG_Ability_Climb);
		 ASC->CancelAbilities(&TagsToCancel);
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

void ATHPlayerCharacter::HandleMoveCompleted(const FInputActionValue& InValue)
{
	if (bIsClimbing)
	{
		ClimbMovementDirection = FVector2D::ZeroVector;
		if (bIsClimbingAndMoving)
		{
			bIsClimbingAndMoving = false;
			Server_UpdateClimbingMovementState(false);
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

void ATHPlayerCharacter::OnRep_ClimbingWallNormal()
{
	if (GetAbilitySystemComponent() && GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_State_Movement_Climbing))
	{
	   const FRotator TargetRotation = FRotationMatrix::MakeFromX(-ClimbingWallNormal).Rotator();
	   SetActorRotation(TargetRotation);
	}
}

void ATHPlayerCharacter::OnSprintPressed(const FInputActionValue&)
{
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

void ATHPlayerCharacter::RequestClimb(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Warning, TEXT("RequestClimb function was called!"));

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
	   if (ASC->HasMatchingGameplayTag(TAG_State_Movement_Climbing))
	   {
		 FGameplayTagContainer TagsToCancel(TAG_Ability_Climb);
		 ASC->CancelAbilities(&TagsToCancel);
	   }
	   
	   else
	   {
		 ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(TAG_Ability_Climb));
	   }
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

void ATHPlayerCharacter::OnClimbingStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
	}
	else
	{
	   LeaveClimbState();
	}
}

void ATHPlayerCharacter::OnSprintingStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	const UTHAttributeSet* AS = Cast<UTHAttributeSet>(GetAbilitySystemComponent()->GetAttributeSet(UTHAttributeSet::StaticClass()));
	if (!AS) return;
	
	bIsSprinting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? AS->GetSprintSpeed() : AS->GetWalkSpeed();
}

void ATHPlayerCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (!bIsSprinting)
	{
	   GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void ATHPlayerCharacter::OnSprintSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (bIsSprinting)
	{
	   GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void ATHPlayerCharacter::OnJumpPowerChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
	   GetCharacterMovement()->JumpZVelocity = Data.NewValue;
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

	GetCharacterMovement()->JumpZVelocity = CurrentJumpPower;

	Super::Jump();
}

void ATHPlayerCharacter::CacheClimbStaminaEffects(const TSubclassOf<UGameplayEffect>& InDrainEffect, const TSubclassOf<UGameplayEffect>& InRegenEffect)
{
	ClimbStaminaDrainEffectClass = InDrainEffect;
	ClimbStaminaRegenEffectClass = InRegenEffect;
}

void ATHPlayerCharacter::SwitchClimbStaminaEffect(bool bShouldRegen)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	if (CurrentClimbStaminaEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(CurrentClimbStaminaEffectHandle);
	}
	
	TSubclassOf<UGameplayEffect> EffectToApply = bShouldRegen ? ClimbStaminaRegenEffectClass : ClimbStaminaDrainEffectClass;

	if (EffectToApply)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectToApply, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			CurrentClimbStaminaEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ATHPlayerCharacter::ClearClimbStaminaEffects()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && CurrentClimbStaminaEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(CurrentClimbStaminaEffectHandle);
	}
	bIsClimbingAndMoving = false;
	ClimbStaminaDrainEffectClass = nullptr;
	ClimbStaminaRegenEffectClass = nullptr;
}

void ATHPlayerCharacter::Server_SetClimbingWallNormal_Implementation(const FVector& InWallNormal)
{
	ClimbingWallNormal = InWallNormal;
	OnRep_ClimbingWallNormal();
}

void ATHPlayerCharacter::Server_UpdateClimbingMovementState_Implementation(bool bNewIsMoving)
{
	bIsClimbingAndMoving = bNewIsMoving;
	SwitchClimbStaminaEffect(!bNewIsMoving); 
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