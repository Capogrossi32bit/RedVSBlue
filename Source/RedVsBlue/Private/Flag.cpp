// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
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
	
}

// Called every frame
void AFlag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFlag::Interact_Implementation(
    AActor* Interactor,
    const FString& InteractorTeam,
    bool InteractorHasFlag
)
{
    GEngine->AddOnScreenDebugMessage(
        -1,
        5.f,
        FColor::Red,
        Team
    );

    GEngine->AddOnScreenDebugMessage(
        -1,
        5.f,
        FColor::Green,
        InteractorTeam
    );

    // Q = Tiene la bandera, P = Mismo equipo que el color de la bandera
    /*
    P|&|Q
    V|V|V
    V|F|F
    F|F|V
    F|F|F
    */

    if (InteractorTeam == Team && InteractorHasFlag && !IsTaken)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Green,
            TEXT("Scored point")
        );

    }

    if (InteractorTeam == Team && !InteractorHasFlag && !IsTaken)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Green,
            TEXT("No Flag")
        );
    }

    if (InteractorTeam != Team && InteractorHasFlag && !IsTaken)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Green,
            TEXT("Already Carrying")
        );
    }
    if (InteractorTeam != Team && !InteractorHasFlag && !IsTaken)
    {
        Character = Cast<ARedVsBlueCharacter>(Interactor);
        Character->SetIsCarrying(true);
        IsTaken = true;
        FlagMesh->SetVisibility(false);
    }
}

