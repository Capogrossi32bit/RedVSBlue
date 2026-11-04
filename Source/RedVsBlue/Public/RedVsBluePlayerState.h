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

	UFUNCTION(BlueprintCallable,Category = "Team")
	ETeam GetTeam() const;

	void SetTeam(ETeam NewTeam);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UFUNCTION()
	void OnRep_Team();
};
