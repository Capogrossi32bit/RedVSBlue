// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "RedVsBlueGameState.h"
#include "Net/UnrealNetwork.h"
#include "RedVsBlueGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "../RedVsBlueCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AFlag::AFlag()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Viewport
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;

	SphereCollision->SetSphereRadius(200.f);

	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetupAttachment(RootComponent);

	// Replicate
	bNetLoadOnClient = true;
	bReplicates = true;
	SetReplicatingMovement(true);
}

// Called when the game starts or when spawned
void AFlag::BeginPlay()
{
	Super::BeginPlay();

    InitialTransform = GetActorTransform();

    OnRep_FlagState();
}


void AFlag::SetFlagState(EFlagState NewState)
{
    if (!HasAuthority())
    {
        return;
    }

    FlagState = NewState;

    OnRep_FlagState();
    ForceNetUpdate();
}


EFlagState AFlag::GetFlagState() const
{
    return FlagState;
}

void AFlag::OnRep_FlagState()
{
    if (!IsValid(FlagMesh) || !IsValid(SphereCollision))
    {
        return;
    }
    const bool bIsCarried = FlagState == EFlagState::Carried;

    FlagMesh->SetVisibility(!bIsCarried);
    SetActorHiddenInGame(false);
    
    SphereCollision->SetCollisionEnabled(bIsCarried ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly);

}

void AFlag::DropFlag(ARedVsBlueCharacter* RequestingCharacter)
{
 
    if (!HasAuthority() || !IsValid(RequestingCharacter))
    {
        return;
    }

    if (RequestingCharacter->GetCarriedFlag() != this)
    {
        return;
    }

    const FVector DropLocation = RequestingCharacter->GetActorLocation() + FVector(0.f, 0.f, 100.f);

    RequestingCharacter->SetCarriedFlag(nullptr);

    Character = nullptr;

    SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

    SetFlagState(EFlagState::Dropped);
}

void AFlag::ReturnToBase()
{
    if (!HasAuthority())
    {
        return;
    }
    //Liberar referencia del portador
    if (IsValid(Character) && Character->GetCarriedFlag() == this)
    {
        Character->SetCarriedFlag(nullptr);
    }
    //Liberar referencia del personaje
    Character = nullptr;

    //Mover la bandera a la base
    SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);

    //Cambiar el estado de la bandera para que refleje su nueva ubicacion
    SetFlagState(EFlagState::AtBase);

}

void AFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFlag, FlagState);
}

void AFlag::Interact_Implementation(AActor* Interactor, const FString& InteractorTeam,bool InteractorHasFlag)
{
    if (!HasAuthority())
    {
        return;
    }

    ARedVsBlueCharacter* InteractingCharacter = Cast<ARedVsBlueCharacter>(Interactor);

    ARedVsBlueGameState* RBGameState = GetWorld()->GetGameState<ARedVsBlueGameState>();
    
    if (!IsValid(RBGameState) || RBGameState->GetMatchPhase() != EMatchPhase::Playing)
    {
        return;
    }

    if (!IsValid(InteractingCharacter))
    {
        return;
    }

    const ETeam CharacterTeam = InteractingCharacter->GetTeam();

    if (CharacterTeam == ETeam::None)
    {
        return;
    }

    const FString CharacterTeamName = CharacterTeam == ETeam::Blue ? TEXT("Blue") : TEXT("Red");

    const bool bSameTeam = CharacterTeamName == Team;

    if (FlagState == EFlagState::Carried)
    {
        return;
    }

    //La bandera es del mismo equipo
    if (bSameTeam)
    {
        if (FlagState == EFlagState::Dropped)
        {
            ReturnToBase();
            return;
        }

        if (FlagState == EFlagState::AtBase)
        {
            ARedVsBlueGameModeBase* RBGameMode = Cast<ARedVsBlueGameModeBase>(UGameplayStatics::GetGameMode(this));

            if (IsValid(RBGameMode))
            {
                RBGameMode->TryCaptureFlag(InteractingCharacter, this);
            }
        }
        return;
    }
    //Si ya lleva una bandera no puede recoger otra
    if (InteractingCharacter->GetCarriedFlag())
    {
        return;
    }
    
    if (FlagState == EFlagState::AtBase || FlagState == EFlagState::Dropped)
    {
        Character = InteractingCharacter;
        InteractingCharacter->SetCarriedFlag(this);
        SetFlagState(EFlagState::Carried);
    }

}

