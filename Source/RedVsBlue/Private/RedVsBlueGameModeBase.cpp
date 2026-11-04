// Fill out your copyright notice in the Description page of Project Settings.


#include "RedVsBlueGameModeBase.h"
#include "RedVsBluePlayerState.h"
#include "RedVsBlueCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "RedVsBlueGameState.h"
#include "Flag.h"
#include "MatchPhase.h"
#include "FlagState.h"
#include "TimerManager.h"
#include "Engine/World.h"

ARedVsBlueGameModeBase::ARedVsBlueGameModeBase()
{
	PlayerStateClass = ARedVsBluePlayerState::StaticClass();

	GameStateClass = ARedVsBlueGameState::StaticClass();
}

void ARedVsBlueGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	StartWaitingPhase();
}

void ARedVsBlueGameModeBase::StartWaitingPhase()
{
	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	RBGameState->SetRemainingTime(FMath::Max(0, WaitingDuration));

	RBGameState->SetMatchPhase(EMatchPhase::Waiting);

	UE_LOG(LogTemp, Warning, TEXT("MATCH: Waiting Phase Started"));

	if (WaitingDuration <= 0)
	{
		StartPlayingPhase();
		return;
	}

	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ARedVsBlueGameModeBase::UpdateMatchTimer, 1.0f, true);

}

void ARedVsBlueGameModeBase::StartPlayingPhase()
{
	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();
	if (!IsValid(RBGameState))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	RBGameState->SetRemainingTime(FMath::Max(0, MatchDuration));

	RBGameState->SetMatchPhase(EMatchPhase::Playing);

	UE_LOG(LogTemp, Warning, TEXT("MATCH: Playing Phase Started"));

	if (MatchDuration <= 0)
	{
		FinishMatch();
		return;
	}

	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ARedVsBlueGameModeBase::UpdateMatchTimer, 1.0f, true);
}

void ARedVsBlueGameModeBase::FinishMatch()
{
	if (!HasAuthority())
	{
		return;
	}

	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	const int32 BlueScore = RBGameState->GetBlueScore();
	const int32 RedScore = RBGameState->GetRedScore();

	ETeam WinningTeam = ETeam::None;

	if (BlueScore > RedScore)
	{
		WinningTeam = ETeam::Blue;
	}

	else if (RedScore > BlueScore)
	{
		WinningTeam = ETeam::Red;
	}

	RBGameState->SetRemainingTime(0);
	RBGameState->SetWinner(WinningTeam);
	RBGameState->SetMatchPhase(EMatchPhase::Finished);

	UE_LOG(LogTemp, Warning, TEXT("MATCH FINISHED | Blue %d | Red %d | Winner %d"), BlueScore, RedScore, static_cast<int32>(WinningTeam));
}

void ARedVsBlueGameModeBase::UpdateMatchTimer()
{
	if (!HasAuthority())
	{
		return;
	}
	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	const int32 NewTime = FMath::Max(0, RBGameState->GetRemainingTime() - 1);

	RBGameState->SetRemainingTime(NewTime);

	if (NewTime > 0)
	{
		return;
	}

	switch (RBGameState->GetMatchPhase())
	{
	case EMatchPhase::Waiting:
		StartPlayingPhase();
		break;

	case EMatchPhase::Playing:
		FinishMatch();
		break;

	case EMatchPhase::Finished:
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		break;
	}
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

bool ARedVsBlueGameModeBase::TryCaptureFlag(ARedVsBlueCharacter* ScoringCharacter, AFlag* HomeFlag)
{
	if (!HasAuthority())
	{
		return false;
	}

	ARedVsBlueGameState* MatchState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(MatchState) || MatchState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return false;
	}

	if (!IsValid(ScoringCharacter) || !IsValid(HomeFlag))
	{
		return false;
	}

	const ETeam PlayerTeam = ScoringCharacter->GetTeam();

	if (PlayerTeam == ETeam::None)
	{
		return false;
	}
	const FString TeamName = PlayerTeam == ETeam::Blue ? TEXT("Blue") : TEXT("Red");

	if (HomeFlag->Team != TeamName)
	{
		return false;
	}

	if (HomeFlag->GetFlagState() != EFlagState::AtBase)
	{
		return false;
	}

	const float Distance = FVector::Dist(ScoringCharacter->GetActorLocation(), HomeFlag->GetActorLocation());

	if (Distance > 250.f)
	{
		return false;
	}
	
	AFlag* EnemyFlag = ScoringCharacter->GetCarriedFlag();

	if (!IsValid(EnemyFlag) || EnemyFlag == HomeFlag)
	{
		return false;
	}

	if (EnemyFlag->Team == TeamName)
	{
		return false;
	}

	if (EnemyFlag->GetFlagState() != EFlagState::Carried)
	{
		return false;
	}

	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return false;
	}

	RBGameState->AddScore(PlayerTeam);

	EnemyFlag->ReturnToBase();

	UE_LOG(LogTemp, Warning, TEXT("CAPTURE SUCCESS | Team: %s | Blue: %d | Red: %d"), *TeamName, RBGameState->GetBlueScore(), RBGameState->GetRedScore());
	return true;
}