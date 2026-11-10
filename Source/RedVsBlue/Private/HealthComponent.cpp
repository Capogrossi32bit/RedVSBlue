#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();

	if (!IsValid(Owner))
	{
		return;
	}

	if (Owner->HasAuthority())
	{
		MaxHealth = FMath::Max(1.f, MaxHealth);
		CurrentHealth = MaxHealth;

		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);

		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
	
}

void UHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

AController* UHealthComponent::GetKillingController() const
{
	return KillingController.Get();
}

float UHealthComponent::GetHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

bool UHealthComponent::IsDead() const
{
	return CurrentHealth<=0.f;
}
void UHealthComponent::Heal(float Amount)
{
	AActor* Owner = GetOwner();

	if (!IsValid(Owner) || !Owner->HasAuthority() || IsDead() || Amount <= 0.f)
	{
		return;
	}
	SetHealth(CurrentHealth + Amount);
}
void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}
;


void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	AActor* Owner = GetOwner();

	if (!IsValid(Owner)||!Owner->HasAuthority())
	{
		return;
	}
	//Si la fuente de daño es el propio jugador, el daño es menor a 0 o el jugador que recive el daño ya esta muerto no hacer nada
	if (DamagedActor != Owner || Damage <= 0.f || IsDead())
	{
		return;
	}

	if (Damage >= CurrentHealth)
	{
		KillingController = InstigatedBy;
	}

	SetHealth(CurrentHealth - Damage);

}

void UHealthComponent::SetHealth(float NewHealth)
{
	AActor* Owner = GetOwner();

	if (!IsValid(Owner))
	{
		return;
	}

	const float OldHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(NewHealth, 0.f, MaxHealth);

	if (FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		return;
	}

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (OldHealth > 0.f && CurrentHealth <= 0.f)
	{
		OnDeath.Broadcast();
	}

	Owner->ForceNetUpdate();
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

