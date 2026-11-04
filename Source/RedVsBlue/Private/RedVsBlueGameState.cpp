// Fill out your copyright notice in the Description page of Project Settings.


#include "RedVsBlueGameState.h"
#include "Net/UnrealNetwork.h"

void ARedVsBlueGameState::AddScore(ETeam ScoringTeam)
{
	if (!HasAuthority())
	{
		return;
	}

	switch (ScoringTeam)
	{
	case ETeam::Blue:
		BlueScore++;
		OnRep_BlueScore();
		break;
	case ETeam::Red:
		RedScore++;
		OnRep_RedScore();
		break;
	default:
		return;
	}
	ForceNetUpdate();
}

//Getter

int32 ARedVsBlueGameState::GetBlueScore() const
{
	return BlueScore;
}

int32 ARedVsBlueGameState::GetRedScore() const
{
	return RedScore;
}

int32 ARedVsBlueGameState::GetRemainingTime() const
{
	return RemainingTime;
}

EMatchPhase ARedVsBlueGameState::GetMatchPhase() const
{
	return MatchPhase;
}

ETeam ARedVsBlueGameState::GetWinner() const
{
	return Winner;
}

//Setter

void ARedVsBlueGameState::SetMatchPhase(EMatchPhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchPhase = NewPhase;
	OnRep_MatchPhase();
	ForceNetUpdate();
}

void ARedVsBlueGameState::SetRemainingTime(int32 NewTime)
{
	if (!HasAuthority())
	{
		return;
	}
	RemainingTime = FMath::Max(0, NewTime);
	OnRep_RemainingTime();
}

void ARedVsBlueGameState::SetWinner(ETeam NewWinner)
{
	if (!HasAuthority())
	{
		return;
	}

	Winner = NewWinner;
	OnRep_Winner();
	ForceNetUpdate();
}

void ARedVsBlueGameState::OnRep_BlueScore()
{
	UE_LOG(LogTemp, Warning, TEXT("Blue Score: %d"), BlueScore);
}

void ARedVsBlueGameState::OnRep_RedScore()
{
	UE_LOG(LogTemp, Warning, TEXT("Red Score: %d"), RedScore);
}

void ARedVsBlueGameState::OnRep_RemainingTime()
{
	UE_LOG(LogTemp, Log, TEXT("REMAINING TIME: %d"), RemainingTime);
}

void ARedVsBlueGameState::OnRep_MatchPhase()
{
	UE_LOG(LogTemp, Warning, TEXT("MATCH PHASE: %d"), static_cast<int32>(MatchPhase));
}

void ARedVsBlueGameState::OnRep_Winner()
{
	const FString WinnerText = Winner == ETeam::Blue ? TEXT("Blue") : Winner == ETeam::Red ? TEXT("Red") : TEXT("Draw");
}

void ARedVsBlueGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARedVsBlueGameState, BlueScore);

	DOREPLIFETIME(ARedVsBlueGameState, RedScore);

	DOREPLIFETIME(ARedVsBlueGameState, MatchPhase);

	DOREPLIFETIME(ARedVsBlueGameState, RemainingTime);

	DOREPLIFETIME(ARedVsBlueGameState, Winner);
}