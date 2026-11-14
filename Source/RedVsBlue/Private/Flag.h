// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamTypes.h"
#include "FlagState.h"
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

	UPROPERTY(ReplicatedUsing = OnRep_FlagState, VisibleAnywhere,BlueprintReadOnly,Category="Flag")
	EFlagState FlagState = EFlagState::AtBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ARedVsBlueCharacter> Character;

	// Functions
	UFUNCTION()
	void OnRep_FlagState();

	UFUNCTION()
	void DropFlag(ARedVsBlueCharacter* RequestingCharacter);

	UFUNCTION()
	void ReturnToBase();

	UFUNCTION(BlueprintCallable, Category = "Flag")
	EFlagState GetFlagState() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Interact_Implementation(AActor* Interactor, const FString& InteractorTeam, bool InteractorHasFlag) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	
	
	FTransform InitialTransform;

	void SetFlagState(EFlagState NewState);

};
