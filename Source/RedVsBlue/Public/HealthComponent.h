#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;

//Evento de recibir y realizar daño
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthChanged, 
	float, NewHealth, 
	float, MaxHealth
);

//Evento de morir
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnDeath
);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REDVSBLUE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	//No es necesario replicar la salud maxima pues la misma no se actualiza durante la partida
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.f;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	UFUNCTION()
	AController* GetKillingController() const;

	//FUNCTIONS

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;		

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

private:

	void SetHealth(float NewHealth);

	UPROPERTY()
	TWeakObjectPtr<AController> KillingController;
};
