// Fill out your copyright notice in the Description page of Project Settings.


#include "RedVsBlueGameModeBase.h"
#include "RedVsBluePlayerState.h"
#include "RedVsBlueCharacter.h"
#include "GameFramework/GameStateBase.h"

ARedVsBlueGameModeBase::ARedVsBlueGameModeBase()
{
	PlayerStateClass = ARedVsBluePlayerState::StaticClass();
}

void ARedVsBlueGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	AssignTeam(NewPlayer);
}

void ARedVsBlueGameModeBase::AssignTeam(APlayerController* NewPlayer)
{
	if (!IsValid(NewPlayer) || !IsValid(GameState))
	{
		return;
	}

	ARedVsBluePlayerState* NewPlayerState = NewPlayer->GetPlayerState<ARedVsBluePlayerState>();

	if (!IsValid(NewPlayerState))
	{
		return;
	}

	int32 BlueCount = 0;
	int32 RedCount = 0;

	for (auto It = AssignedTeams.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		if (It.Value() == ETeam::Blue)
		{
			BlueCount++;
		}
		else if (It.Value() == ETeam::Red)
		{
			RedCount++;
		}
	}

	if (AssignedTeams.Contains(NewPlayerState))
	{
		return;
	}

	ETeam AssignedTeam =
		BlueCount <= RedCount ? ETeam::Blue : ETeam::Red;

	NewPlayerState->SetTeam(AssignedTeam);
	
	AssignedTeams.Add(NewPlayerState, AssignedTeam);

}



