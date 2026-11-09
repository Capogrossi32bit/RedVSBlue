#pragma once

#include "CoreMinimal.h"
#include "TeamTypes.h"
#include "TimerManager.h"
#include "GameFramework/GameModeBase.h"
#include "RedVsBlueGameModeBase.generated.h"

class APlayerState;
class APlayerController;
class ARedVsBlueCharacter;
class AFlag;

UCLASS()
class REDVSBLUE_API ARedVsBlueGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	ARedVsBlueGameModeBase();

	bool TryCaptureFlag(ARedVsBlueCharacter* ScoringCharacter, AFlag* HomeFlag);

	void HandlePlayerDeath(ARedVsBlueCharacter* DeadCharacter);

protected:

	TMap<TWeakObjectPtr<APlayerState>, ETeam> AssignedTeams;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 WaitingDuration = 15;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 MatchDuration = 300;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle MatchTimerHandle;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Respawn", meta = (ClampMin = "0.0"))
	float RespawnDelay = 5.f;

	UFUNCTION()
	void RespawnPlayer(TWeakObjectPtr<APlayerController> PlayerController);

	UFUNCTION()
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void StartWaitingPhase();

	UFUNCTION()
	void StartPlayingPhase();

	UFUNCTION()
	void FinishMatch();

	UFUNCTION()
	void UpdateMatchTimer();

	UFUNCTION()
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION()
	void AssignTeam(APlayerController* NewPlayer);
	
	UFUNCTION()
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UFUNCTION()
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
};
