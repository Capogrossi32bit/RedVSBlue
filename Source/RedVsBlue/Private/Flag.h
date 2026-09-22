// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractInterface.h"
#include "Flag.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ARedVsBlueCharacter;

UCLASS()
class AFlag : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:


	// Sets default values for this actor's properties
	AFlag();

	// Properties

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> FlagMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsTaken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ARedVsBlueCharacter> Character;

	// Functions

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation(AActor* Interactor, const FString& InteractorTeam, bool InteractorHasFlag) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	
	

};
