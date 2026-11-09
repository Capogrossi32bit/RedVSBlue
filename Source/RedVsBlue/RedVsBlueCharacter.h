// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TeamTypes.h"
#include "RedVsBlueCharacter.generated.h"

class UHealthComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class AFlag;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ARedVsBlueCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MouseMoveMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;

	/** Interact Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	/** Drop Flag Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DropFlagAction;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedFlag,EditAnywhere,BlueprintReadWrite, Category = "Flag")
	TObjectPtr<AFlag> CarriedFlag = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flag")
	UStaticMeshComponent* FlagMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Combat")
	float WeaponDamage = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float WeaponRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float FireInterval = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 MaxAmmo = 30;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 CurrentAmmo = 30;

	float LastFireTime = -1000.f;

protected:

	UFUNCTION()
	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	UFUNCTION()
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	
	UFUNCTION()
	virtual void PawnClientRestart() override;

	UFUNCTION()
	void HandleCharacterDeath();

	UFUNCTION()
	void OnRep_CurrentAmmo();
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);


public:

	/** Constructor */
	ARedVsBlueCharacter();

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION()
	void Fire();

	UFUNCTION(Server,Reliable)
	void Server_Fire();

	UFUNCTION(Client,Reliable)
	void Client_HitConfirmed();

	UFUNCTION(NetMulticast,Unreliable)
	void Multicast_FireEffects(FVector TraceStart, FVector TraceEnd, bool bHitSomething);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetMaxAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Getter")
	bool GetIsCarrying();

	UFUNCTION(BlueprintCallable, Category= "Flag")
	AFlag* GetCarriedFlag() const;

	UFUNCTION(BlueprintCallable, Category = "Flag")
	void SetCarriedFlag(AFlag* NewFlag);

	UFUNCTION()
	void DropCarriedFlag();

	UFUNCTION(Server,Reliable)
	void Server_DropFlag();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Team")
	ETeam GetTeam() const;

	UFUNCTION()
	void OnRep_CarriedFlag();

	void InteractOtherActor();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:

	UPROPERTY()
	TObjectPtr<AActor> OverlapActor;
};

