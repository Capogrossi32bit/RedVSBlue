// Copyright Epic Games, Inc. All Rights Reserved.

#include "RedVsBlueCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Flag.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "InteractInterface.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "RedVsBlue.h"
#include "RedVsBlueGameState.h"
#include "RedVsBluePlayerState.h"

ARedVsBlueCharacter::ARedVsBlueCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bReplicates = true;

	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarriedFlagMesh"));
	FlagMesh->SetupAttachment(GetMesh(), TEXT("FlagSocket"));
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetVisibility(false);
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}


void ARedVsBlueCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARedVsBlueCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ARedVsBlueCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARedVsBlueCharacter::Look);

		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ARedVsBlueCharacter::InteractOtherActor);
		EnhancedInputComponent->BindAction(DropFlagAction, ETriggerEvent::Started, this, &ARedVsBlueCharacter::DropCarriedFlag);
	}
	else
	{
		UE_LOG(LogRedVsBlue, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARedVsBlueCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ARedVsBlueCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

ETeam ARedVsBlueCharacter::GetTeam() const
{
	const ARedVsBluePlayerState* RBPlayerState = GetPlayerState<ARedVsBluePlayerState>();

	if (!IsValid(RBPlayerState))
    {
        return ETeam::None;
    }

	return RBPlayerState->GetTeam();
}

void ARedVsBlueCharacter::OnRep_CarriedFlag()
{
	if (!IsValid(FlagMesh))
	{
		return;
	}

	if (!IsValid(CarriedFlag))
	{
		FlagMesh->SetVisibility(false);
		FlagMesh->SetStaticMesh(nullptr);
		return;
	}
	if (!IsValid(CarriedFlag->FlagMesh))
	{
		return;
	}
	UStaticMeshComponent* SourceMesh = CarriedFlag->FlagMesh;

	FlagMesh->SetStaticMesh(SourceMesh->GetStaticMesh());

	for (int32 Index = 0; Index < SourceMesh->GetNumMaterials();Index++)
	{
		FlagMesh->SetMaterial(Index, SourceMesh->GetMaterial(Index));
	}

	FlagMesh->SetVisibility(true);
}

void ARedVsBlueCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ARedVsBlueCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ARedVsBlueCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ARedVsBlueCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

bool ARedVsBlueCharacter::GetIsCarrying()
{
	return IsValid(CarriedFlag);
}

AFlag* ARedVsBlueCharacter::GetCarriedFlag() const
{
	return CarriedFlag.Get();
}

void ARedVsBlueCharacter::SetCarriedFlag(AFlag* NewFlag)
{
	if (!HasAuthority())
	{
		return;
	}

	CarriedFlag = NewFlag;

	OnRep_CarriedFlag();
	ForceNetUpdate();
}

void ARedVsBlueCharacter::DropCarriedFlag()
{
	Server_DropFlag();
}

void ARedVsBlueCharacter::Server_DropFlag_Implementation()
{
	ARedVsBlueGameState* RBGameState = GetWorld()->GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState) || RBGameState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return;
	}

	if (!IsValid(CarriedFlag))
	{
		return;
	}
	CarriedFlag->DropFlag(this);
}

void ARedVsBlueCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARedVsBlueCharacter, CarriedFlag);
}

void ARedVsBlueCharacter::Server_Interact_Implementation(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || !TargetActor->Implements<UInteractInterface>()) 
	{
		return;
	}
	const float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

	if (Distance > 250.f)
	{
		return;
	}

	const ETeam CurrentTeam = GetTeam();

	if (CurrentTeam == ETeam::None) 
	{
		return;
	}

	const FString TeamString = CurrentTeam == ETeam::Blue ? TEXT("Blue") : TEXT("Red");

	IInteractInterface::Execute_Interact(TargetActor, this, TeamString, GetIsCarrying());
}

void ARedVsBlueCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	OverlapActor = OtherActor;
}

void ARedVsBlueCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	if (OverlapActor == OtherActor) 
	{
		OverlapActor = nullptr;
	}
}

void ARedVsBlueCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!IsValid(PC) || !PC->IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();

	if (!IsValid(LocalPlayer))
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	if (IsValid(Subsystem))
	{
		if (IsValid(DefaultMappingContext))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		if (IsValid(MouseMoveMappingContext))
		{
			Subsystem->AddMappingContext(MouseMoveMappingContext, 1);
		}

	}
}


void ARedVsBlueCharacter::InteractOtherActor()
{
	if (IsValid(OverlapActor) && OverlapActor->Implements<UInteractInterface>())
	{
		Server_Interact(OverlapActor.Get());
	}
}