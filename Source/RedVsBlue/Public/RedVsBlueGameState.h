#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TeamTypes.h"
#include "MatchPhase.h"
#include "RedVsBlueGameState.generated.h"

UCLASS()
class REDVSBLUE_API ARedVsBlueGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UPROPERTY(ReplicatedUsing = OnRep_BlueScore, BlueprintReadOnly, Category = "Score")
	int32 BlueScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_RedScore, BlueprintReadOnly, Category = "Score")
	int32 RedScore = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly, Category = "Match")
	int32 RemainingTime = 15;

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly,Category = "Match")
	EMatchPhase MatchPhase = EMatchPhase::Waiting;

	UPROPERTY(ReplicatedUsing = OnRep_Winner, BlueprintReadOnly, Category = "Match")
	ETeam Winner = ETeam::None;

	//Getters

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetBlueScore() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetRedScore() const;

	UFUNCTION(BlueprintCallable, Category = "Match")
	int32 GetRemainingTime() const;

	UFUNCTION(BlueprintCallable, Category = "Match")
	EMatchPhase GetMatchPhase() const;

	UFUNCTION(BlueprintCallable, Category = "Match")
	ETeam GetWinner() const;

	//Setters

	UFUNCTION()
	void SetMatchPhase(EMatchPhase NewPhase);

	UFUNCTION()
	void SetRemainingTime(int32 NewTime);

	UFUNCTION()
	void SetWinner(ETeam NewWinner);

	UFUNCTION()
	void AddScore(ETeam ScoringTeam);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UFUNCTION()
	void OnRep_BlueScore();

	UFUNCTION()
	void OnRep_RedScore();

	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_Winner();
};
