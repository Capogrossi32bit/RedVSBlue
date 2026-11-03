// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Net/UnrealNetwork.h"
#include "../RedVsBlueCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AFlag::AFlag()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
    FlagMesh->SetVisibility(!IsTaken);
	
}

// Called every frame
void AFlag::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AFlag::OnRep_IsTaken()
{
    FlagMesh->SetVisibility(!IsTaken);
}

void AFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFlag, IsTaken);
}

void AFlag::Interact_Implementation(AActor* Interactor, const FString& InteractorTeam,bool InteractorHasFlag)
{
    if (!HasAuthority())
    {
        return;
    }

    ARedVsBlueCharacter* InteractingCharacter = Cast<ARedVsBlueCharacter>(Interactor);

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

    //La bandera es del mismo equipo
    if (CharacterTeamName == Team)
    {
        return;
    }

    //La bandera ya fue tomada
    if (IsTaken)
    {
        return;
    }

    //Ya transporta una bandera
    if (InteractingCharacter->GetIsCarrying())
    {
        return;
    }

    InteractingCharacter->SetCarriedFlag(this);
    IsTaken = true;

    OnRep_IsTaken();
    ForceNetUpdate();
}

