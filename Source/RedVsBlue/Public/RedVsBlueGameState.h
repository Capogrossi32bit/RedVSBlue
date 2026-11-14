#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TeamTypes.h"
#include "MatchPhase.h"
#include "RedVsBlueGameState.generated.h"

class USoundBase;

UCLASS()
class REDVSBLUE_API ARedVsBlueGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
	int32 BlueScore = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
	int32 RedScore = 0;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	int32 RemainingTime = 15;

	UPROPERTY(Replicated, BlueprintReadOnly,Category = "Match")
	EMatchPhase MatchPhase = EMatchPhase::Waiting;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	ETeam Winner = ETeam::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	TObjectPtr<USoundBase> CaptureSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	TObjectPtr<USoundBase> DeathSound;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayCaptureSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathSound(FVector DeathLocation);

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

};
