#include "RedVsBlueCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Flag.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "HealthComponent.h"
#include "InteractInterface.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "RedVsBlue.h"
#include "RedVsBlueGameState.h"
#include "../Public/RedVsBlueGameModeBase.h"
#include "RedVsBluePlayerState.h"
#include "Sound/SoundBase.h"

ARedVsBlueCharacter::ARedVsBlueCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
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

	CameraBoom->TargetOffset = FVector(0.f, 0.f, 65.f);
	CameraBoom->SocketOffset = FVector(0.f, 85.f, 35.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bReplicates = true;

	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarriedFlagMesh"));
	FlagMesh->SetupAttachment(GetMesh(), TEXT("FlagSocket"));
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetVisibility(false);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	CurrentAmmo = MaxAmmo;
	ReserveAmmo = MaxReserveAmmo;
	LastFireTime = -1000.f;
	bIsReloading = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ARedVsBlueCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && IsValid(HealthComponent))
	{
		CurrentAmmo = MaxAmmo;
		ReserveAmmo = MaxReserveAmmo;
		bIsReloading = false;
		HealthComponent->OnDeath.AddDynamic(this, &ARedVsBlueCharacter::HandleCharacterDeath);
		BindTeamMaterial();
	}
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

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ARedVsBlueCharacter::InteractOtherActor);

		//Drop Flag (Testing)
		EnhancedInputComponent->BindAction(DropFlagAction, ETriggerEvent::Started, this, &ARedVsBlueCharacter::DropCarriedFlag);

		// Shooting
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ARedVsBlueCharacter::Fire);

		// Reloading
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ARedVsBlueCharacter::Reload);
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

void ARedVsBlueCharacter::HandleCharacterDeath()
{
	if (!HasAuthority())
	{
		return;
	}
	
	CancelReload();

	if (IsValid(CarriedFlag))
	{
		CarriedFlag->DropFlag(this);
	}

	GetCharacterMovement()->DisableMovement();

	ARedVsBlueGameModeBase* RBGameMode = Cast<ARedVsBlueGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (IsValid(RBGameMode))
	{
		RBGameMode->HandlePlayerDeath(this);
	}
}



void ARedVsBlueCharacter::InteractOtherActor()
{
	if (IsValid(OverlapActor) && OverlapActor->Implements<UInteractInterface>())
	{
		Server_Interact(OverlapActor.Get());
	}
}

// ========================= TEAM =========================

ETeam ARedVsBlueCharacter::GetTeam() const
{
	const ARedVsBluePlayerState* RBPlayerState = GetPlayerState<ARedVsBluePlayerState>();

	if (!IsValid(RBPlayerState))
	{
		return ETeam::None;
	}

	return RBPlayerState->GetTeam();
}

void ARedVsBlueCharacter::UpdateTeamMaterial()
{
	if(!IsValid(GetMesh()))
	{
		return;
	}

	const ETeam CurrentTeam = GetTeam();

	if (CurrentTeam == ETeam::None)
	{
		return;
	}

	if (!IsValid(TeamMaterialInstance))
	{
		TeamMaterialInstance = GetMesh()->CreateDynamicMaterialInstance(TeamMaterialIndex);
	}

	if(!IsValid(TeamMaterialInstance))
	{
		return;
	}

	const FLinearColor NewColor = CurrentTeam == ETeam::Blue ? BlueTeamColor : RedTeamColor;

	TeamMaterialInstance->SetVectorParameterValue(TEXT("TeamColor"), NewColor);
}

void ARedVsBlueCharacter::BindTeamMaterial()
{
	ARedVsBluePlayerState* RBPlayerState = GetPlayerState<ARedVsBluePlayerState>();

	if (!IsValid(RBPlayerState))
	{
		return;
	}

	RBPlayerState->OnTeamChanged.RemoveAll(this);

	RBPlayerState->OnTeamChanged.AddUObject(this, &ARedVsBlueCharacter::UpdateTeamMaterial);

	UpdateTeamMaterial();
}


void ARedVsBlueCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	BindTeamMaterial();
}


// ========================= FLAG =========================

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

// ========================= COMBAT =========================


void ARedVsBlueCharacter::PlayCombatMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!IsValid(Montage) || !IsValid(GetMesh()))
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (!IsValid(AnimInstance))
	{
		return;
	}

	AnimInstance->Montage_Play(Montage, PlayRate);
}

void ARedVsBlueCharacter::FinishReload()
{
	if (!HasAuthority() || !bIsReloading)
	{
		return;
	}

	ARedVsBlueGameState* RBGameState = GetWorld()->GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState) || RBGameState->GetMatchPhase() != EMatchPhase::Playing || !IsValid(HealthComponent) || HealthComponent->IsDead())
	{
		CancelReload();
		return;
	}

	const int32 AmmoNeeded = FMath::Max(0, MaxAmmo - CurrentAmmo);
	const int32 AmmoToReload = FMath::Min(AmmoNeeded, ReserveAmmo);

	//Lo agrego a la municion actual
	CurrentAmmo += AmmoToReload;

	//Lo quito de las reservas
	ReserveAmmo -= AmmoToReload;

	bIsReloading = false;
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

	OnRep_CurrentAmmo();
	OnRep_ReserveAmmo();
	OnRep_IsReloading();

	ForceNetUpdate();

	UE_LOG(LogTemp, Warning, TEXT("RELOAD COMPLETE | Ammo: %d | Reserve: %d"), CurrentAmmo, ReserveAmmo);
}

void ARedVsBlueCharacter::CancelReload()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

	if (bIsReloading)
	{
		bIsReloading = false;
		OnRep_IsReloading();
		ForceNetUpdate();
	}
}


float ARedVsBlueCharacter::GetAimPitch() const
{
	const FRotator AimRotation = GetBaseAimRotation();

	const FRotator CharacterRotation = GetActorRotation();

	const FRotator DeltaRotation = (AimRotation - CharacterRotation).GetNormalized();

	return FMath::Clamp(DeltaRotation.Pitch / 60.f, -1.f, 1.f);
}

void ARedVsBlueCharacter::Fire()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	Server_Fire();
}

void ARedVsBlueCharacter::Reload()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	Server_Reload();
}

int32 ARedVsBlueCharacter::GetReserveAmmo() const
{
	return ReserveAmmo;
}

bool ARedVsBlueCharacter::IsReloading() const
{
	return bIsReloading;
}

int32 ARedVsBlueCharacter::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 ARedVsBlueCharacter::GetMaxAmmo() const
{
	return MaxAmmo;
}

// ========================= RPC =========================

void ARedVsBlueCharacter::Server_Interact_Implementation(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || !TargetActor->Implements<UInteractInterface>())
	{
		return;
	}

	if (!IsValid(HealthComponent) || HealthComponent->IsDead())
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

void ARedVsBlueCharacter::Multicast_FireEffects_Implementation(FVector TraceStart, FVector TraceEnd, bool bHitSomething)
{
	PlayCombatMontage(FireMontage);
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Yellow, false, 0.3f, 0, 1.5f);
	if (bHitSomething)
	{
		DrawDebugSphere(GetWorld(), TraceEnd, 12.f, 8, FColor::Red, false, 0.3f);
	}
	if (IsValid(FireSound) && GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, TraceStart);
	}
}

void ARedVsBlueCharacter::Server_Fire_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	// Validar Game Phase
	ARedVsBlueGameState* RBGameState = GetWorld()->GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState) || RBGameState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	// Validar Player State

	if (!IsValid(HealthComponent) || HealthComponent->IsDead() || CurrentAmmo <= 0 || bIsReloading)
	{
		return;
	}

	if (GetTeam() == ETeam::None)
	{
		return;
	}

	//Validar cadencia de fuego

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastFireTime < FireInterval)
	{
		return;
	}
	LastFireTime = CurrentTime;
	CurrentAmmo--;
	OnRep_CurrentAmmo();

	AController* ShooterController = GetController();

	if (!IsValid(ShooterController))
	{
		return;
	}

	//Camara
	FVector CameraLocation;
	FRotator CameraRotation;
	ShooterController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector CameraTraceStart = CameraLocation;
	const FVector CameraTraceEnd = CameraTraceStart + (CameraRotation.Vector() * WeaponRange);

	FHitResult CameraHitResult;
	FCollisionQueryParams CameraQueryParams;
	CameraQueryParams.AddIgnoredActor(this);
	CameraQueryParams.bTraceComplex = false;

	const bool bCameraHitSomething = GetWorld()->LineTraceSingleByChannel(CameraHitResult, CameraTraceStart, CameraTraceEnd, ECC_Visibility, CameraQueryParams);

	const FVector AimPoint = bCameraHitSomething ? CameraHitResult.ImpactPoint : CameraTraceEnd;


	// Pistola

	const FName MuzzleSocketName(TEXT("MuzzleSocket"));

	const FVector MuzzleLocation = WeaponMesh->DoesSocketExist(MuzzleSocketName) ? WeaponMesh->GetSocketLocation(MuzzleSocketName) : WeaponMesh->GetComponentLocation();

	const FVector ShotDirection = (AimPoint - MuzzleLocation).GetSafeNormal();

	if (ShotDirection.IsNearlyZero())
	{
		return;
	}

	const FVector WeaponTraceEnd = MuzzleLocation + ShotDirection * WeaponRange;

	FHitResult WeaponHit;

	const bool bWeaponHit = World->LineTraceSingleByChannel(WeaponHit,MuzzleLocation,WeaponTraceEnd,ECC_Visibility,CameraQueryParams);

	const FVector FinalEnd = bWeaponHit ? WeaponHit.ImpactPoint : WeaponTraceEnd;

	if (bWeaponHit)
	{
		ARedVsBlueCharacter* HitCharacter = Cast<ARedVsBlueCharacter>(WeaponHit.GetActor());

		// SI el personaje es valido, y no se esta disparando a si mismo, a un compañero de equipo o a alguien que no pertenezca a un equipo
		if (IsValid(HitCharacter) && HitCharacter != this && HitCharacter->GetTeam() != ETeam::None && HitCharacter->GetTeam() != GetTeam())
		{
			UHealthComponent* TargetHealth = HitCharacter->FindComponentByClass<UHealthComponent>();
			if (IsValid(TargetHealth) && !TargetHealth->IsDead())
			{
				const float AppliedDamage = UGameplayStatics::ApplyDamage(HitCharacter, WeaponDamage, ShooterController, this, UDamageType::StaticClass());
				if (AppliedDamage > 0.f)
				{
					Client_HitConfirmed();
				}
			}
		}
	}

	Multicast_FireEffects(MuzzleLocation, FinalEnd, bWeaponHit);
}

void ARedVsBlueCharacter::Client_HitConfirmed_Implementation()
{
	if (IsValid(HitConfirmedSound))
	{
		UGameplayStatics::PlaySound2D(this, HitConfirmedSound);
	}
	UE_LOG(LogTemp, Warning, TEXT("Hit Confirmed!"));
}

void ARedVsBlueCharacter::Server_Reload_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	ARedVsBlueGameState* RBGameState = GetWorld()->GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState) || RBGameState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return;
	}

	if (!IsValid(HealthComponent) || HealthComponent->IsDead())
	{
		return;
	}

	if (bIsReloading)
	{
		return;
	}

	if (CurrentAmmo >= MaxAmmo)
	{
		return;
	}

	if (ReserveAmmo <= 0)
	{
		return;
	}

	bIsReloading = true;
	OnRep_IsReloading();
	ForceNetUpdate();

	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &ARedVsBlueCharacter::FinishReload, FMath::Max(0.1f, ReloadDuration), false);

	UE_LOG(LogTemp, Warning, TEXT("RELOAD STARTED | Ammo: %d | Reserve: %d"), CurrentAmmo, ReserveAmmo);
}

//  ========================= REPLICATION =========================


void ARedVsBlueCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARedVsBlueCharacter, CarriedFlag);
	DOREPLIFETIME_CONDITION(ARedVsBlueCharacter, CurrentAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ARedVsBlueCharacter, ReserveAmmo, COND_OwnerOnly);
	DOREPLIFETIME(ARedVsBlueCharacter, bIsReloading);
}

void ARedVsBlueCharacter::OnRep_ReserveAmmo()
{
	UE_LOG(LogTemp, Log, TEXT("RESERVE AMMO: %d"), ReserveAmmo);
}

void ARedVsBlueCharacter::OnRep_IsReloading()
{

	if (bIsReloading)
	{
		if (IsValid(ReloadMontage))
		{
			const float MontageLength = ReloadMontage->GetPlayLength();

			const float PlayRate = MontageLength / FMath::Max(0.1f, ReloadDuration);

			if (IsValid(ReloadSound) && GetNetMode() != NM_DedicatedServer)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
			}

			PlayCombatMontage(ReloadMontage, PlayRate);
		}

		else
		{
			if (IsValid(GetMesh()))
			{
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (IsValid(AnimInstance) && IsValid(ReloadMontage) && AnimInstance->Montage_IsPlaying(ReloadMontage))
				{
					AnimInstance->Montage_Stop(0.15f, ReloadMontage);
				}
			}
		}

	}
	UE_LOG(LogTemp, Log, TEXT("RELOADING: %s"), bIsReloading ? TEXT("YES") : TEXT("NO"));
}

void ARedVsBlueCharacter::OnRep_CurrentAmmo()
{
	UE_LOG(LogTemp, Log, TEXT("Ammo: %d / %d"), CurrentAmmo, MaxAmmo);
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

void ARedVsBlueCharacter::OnRep_PlayerState()
{
	Super::OnRep_AttachmentReplication();
	BindTeamMaterial();
}
