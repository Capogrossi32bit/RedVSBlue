// Fill out your copyright notice in the Description page of Project Settings.


#include "RedVsBlueGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
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
		break;
	case ETeam::Red:
		RedScore++;
		break;
	default:
		return;
	}
	ForceNetUpdate();
	Multicast_PlayCaptureSound();
}

//Getter

void ARedVsBlueGameState::Multicast_PlayCaptureSound_Implementation()
{
	if (IsValid(CaptureSound) && GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::PlaySound2D(this, CaptureSound);
	}
}

void ARedVsBlueGameState::Multicast_PlayDeathSound_Implementation(FVector DeathLocation)
{
	if (IsValid(DeathSound) && GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, DeathLocation);
	}
}

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
	ForceNetUpdate();
}

void ARedVsBlueGameState::SetRemainingTime(int32 NewTime)
{
	if (!HasAuthority())
	{
		return;
	}
	RemainingTime = FMath::Max(0, NewTime);

}

void ARedVsBlueGameState::SetWinner(ETeam NewWinner)
{
	if (!HasAuthority())
	{
		return;
	}

	Winner = NewWinner;
	ForceNetUpdate();
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