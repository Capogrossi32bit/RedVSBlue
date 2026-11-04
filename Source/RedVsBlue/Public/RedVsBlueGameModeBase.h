#pragma once

#include "CoreMinimal.h"
#include "TeamTypes.h"
#include "TimerManager.h"
#include "GameFramework/GameModeBase.h"
#include "RedVsBlueGameModeBase.generated.h"

class APlayerState;
class ARedVsBlueCharacter;
class AFlag;

UCLASS()
class REDVSBLUE_API ARedVsBlueGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	ARedVsBlueGameModeBase();

	bool TryCaptureFlag(ARedVsBlueCharacter* ScoringCharacter, AFlag* HomeFlag);

protected:

	TMap<TWeakObjectPtr<APlayerState>, ETeam> AssignedTeams;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 WaitingDuration = 15;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 MatchDuration = 300;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle MatchTimerHandle;

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
	
};
