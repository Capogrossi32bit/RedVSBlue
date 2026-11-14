#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TeamTypes.h"
#include "RedVsBluePlayerState.generated.h"

UCLASS()
class REDVSBLUE_API ARedVsBluePlayerState : public APlayerState
{
	GENERATED_BODY()
public:

	UPROPERTY(ReplicatedUsing = OnRep_Team, BlueprintReadOnly, Category = "Team")
	ETeam Team = ETeam::None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Statistics")
	int32 Kills = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Statistics")
	int32 Deaths = 0;

	UFUNCTION(BlueprintCallable,Category = "Team")
	ETeam GetTeam() const;

	UFUNCTION()
	void SetTeam(ETeam NewTeam);

	UFUNCTION()
	void AddKill();

	UFUNCTION()
	void AddDeath();

	UFUNCTION()
	int32 GetKills() const;

	UFUNCTION()
	int32 GetDeaths() const;

	FSimpleMulticastDelegate OnTeamChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UFUNCTION()
	void OnRep_Team();

};
