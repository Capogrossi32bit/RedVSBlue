// Fill out your copyright notice in the Description page of Project Settings.


#include "RedVsBluePlayerState.h"
#include "Net/UnrealNetwork.h"

ETeam ARedVsBluePlayerState::GetTeam() const
{
	return Team;
}

void ARedVsBluePlayerState::SetTeam(ETeam NewTeam)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Team == NewTeam)
	{
		return;
	}
	Team = NewTeam;
	OnRep_Team();
	ForceNetUpdate();
}

int32 ARedVsBluePlayerState::GetDeaths() const
{
	return Deaths;
}

void ARedVsBluePlayerState::AddKill()
{
    if (!HasAuthority())
    {
        return;
    }

    ++Kills;
    OnRep_Kills();
    ForceNetUpdate();
}

void ARedVsBluePlayerState::AddDeath()
{
    if (!HasAuthority())
    {
        return;
    }

    ++Deaths;
    OnRep_Deaths();
    ForceNetUpdate();
}

int32 ARedVsBluePlayerState::GetKills() const
{
    return Kills;
}


void ARedVsBluePlayerState::OnRep_Kills()
{
	UE_LOG(LogTemp, Warning, TEXT("PLAYER STATS | %s | Kills: %d"), *GetPlayerName(), Kills);
}

void ARedVsBluePlayerState::OnRep_Deaths()
{
	UE_LOG(LogTemp, Warning, TEXT("PLAYER STATS | %s | Deaths: %d"), *GetPlayerName(), Deaths);
}

void ARedVsBluePlayerState::OnRep_Team()
{
	const FString TeamText = StaticEnum<ETeam>()->GetNameStringByValue(static_cast<int64>(Team));
	OnTeamChanged.Broadcast();

}


void ARedVsBluePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARedVsBluePlayerState, Team);
	DOREPLIFETIME(ARedVsBluePlayerState, Kills);
	DOREPLIFETIME(ARedVsBluePlayerState, Deaths);
}



