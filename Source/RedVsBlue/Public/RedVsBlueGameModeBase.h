// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamTypes.h"
#include "GameFramework/GameModeBase.h"
#include "RedVsBlueGameModeBase.generated.h"

class APlayerState;
/**
 * 
 */
UCLASS()
class REDVSBLUE_API ARedVsBlueGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	ARedVsBlueGameModeBase();

protected:

	TMap<TWeakObjectPtr<APlayerState>, ETeam> AssignedTeams;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	void AssignTeam(APlayerController* NewPlayer);
	
};
